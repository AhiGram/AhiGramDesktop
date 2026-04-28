/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_settings_network.h"
#include "ahigram/core/ahi_storage.h"
#include "ahigram/ahi_lang.h"

#include "apiwrap.h"
#include "boxes/filters/edit_filter_chats_list.h"
#include "boxes/filters/edit_filter_chats_preview.h"
#include "boxes/peer_list_controllers.h"
#include "data/data_session.h"
#include "data/data_channel.h"
#include "data/data_folder.h"
#include "history/history.h"
#include "lang/lang_keys.h"
#include "main/main_session.h"
#include "ui/widgets/buttons.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/vertical_list.h"
#include "window/window_session_controller.h"
#include "styles/style_ahi_settings.h"
#include "styles/style_settings.h"

namespace Settings {
namespace {
class PublicChannelsController final : public ChatsListBoxController {
public:
	PublicChannelsController(
		not_null<Main::Session*> session,
		const base::flat_set<not_null<History*>> &selected);

	Main::Session &session() const override;
	void prepareViewHook() override;
	void rowClicked(not_null<PeerListRow*> row) override;
	std::unique_ptr<Row> createRow(not_null<History*> history) override;

private:
	bool isValidPublicChannel(not_null<History*> history) const;

	const not_null<Main::Session*> _session;
	base::flat_set<not_null<History*>> _selected;
};

PublicChannelsController::PublicChannelsController(
	not_null<Main::Session*> session,
	const base::flat_set<not_null<History*>> &selected)
: ChatsListBoxController(session)
, _session(session)
, _selected(selected) {
}

Main::Session &PublicChannelsController::session() const {
	return *_session;
}

bool PublicChannelsController::isValidPublicChannel(
		not_null<History*> history) const {
	const auto channel = history->peer->asChannel();
	return channel && !channel->username().isEmpty();
}

void PublicChannelsController::prepareViewHook() {
	const auto addIfValid = [&](not_null<History*> history) {
		if (!isValidPublicChannel(history)) {
			return;
		}
		if (delegate()->peerListFindRow(history->peer->id.value)) {
			return;
		}
		delegate()->peerListAppendRow(createRow(history));
		if (_selected.contains(history)) {
			delegate()->peerListSetRowChecked(
				delegate()->peerListFindRow(history->peer->id.value),
				true);
		}
	};

	const auto mainList = _session->data().chatsList()->indexed();
	for (const auto &row : mainList->all()) {
		if (const auto history = row->key().history()) {
			addIfValid(history);
		}
	}

	if (const auto folder = _session->data().folderLoaded(Data::Folder::kId)) {
		const auto folderList = folder->chatsList()->indexed();
		for (const auto &row : folderList->all()) {
			if (const auto history = row->key().history()) {
				addIfValid(history);
			}
		}
	}

	for (const auto &history : _selected) {
		if (!isValidPublicChannel(history)) {
			continue;
		}
		if (!delegate()->peerListFindRow(history->peer->id.value)) {
			delegate()->peerListAppendRow(createRow(history));
			delegate()->peerListSetRowChecked(
				delegate()->peerListFindRow(history->peer->id.value),
				true);
		}
	}

	delegate()->peerListRefreshRows();
}

void PublicChannelsController::rowClicked(not_null<PeerListRow*> row) {
	delegate()->peerListSetRowChecked(row, !row->checked());
}

std::unique_ptr<ChatsListBoxController::Row> PublicChannelsController::createRow(
		not_null<History*> history) {
	if (!isValidPublicChannel(history)) {
		return nullptr;
	}
	return std::make_unique<Row>(history);
}

} // namespace

AhiNetworkSettings::AhiNetworkSettings(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section<AhiNetworkSettings>(parent, controller)
, _controller(controller) {
	setupContent();
}

rpl::producer<QString> AhiNetworkSettings::title() {
	return AhiGram::trReactive(u"ahigram_bypass_title"_q);
}

void AhiNetworkSettings::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	auto &settings = AhiGram::Storage::Settings::Instance().data();

	Ui::AddSkip(content);
	Ui::AddSubsectionTitle(content, AhiGram::trReactive(u"ahigram_auto_proxy"_q));

	const auto bypassButton = content->add(
		object_ptr<Ui::SettingsButton>(
			content,
			AhiGram::trReactive(u"ahigram_bypass_slowdown_title"_q),
			st::ahiSettingsButtonNoIcon));

	bypassButton->toggleOn(settings.ahiBypass.value());

	bypassButton->toggledChanges(
	) | rpl::on_next([=, &settings](bool toggled) {
		settings.ahiBypass.force_assign(toggled);
	}, content->lifetime());

	Ui::AddSkip(content);
	Ui::AddDivider(content);
	Ui::AddSkip(content);

	Ui::AddSubsectionTitle(content, AhiGram::trReactive(u"ahigram_proxy_channels_title"_q));

	const auto session = &_controller->session();

	const auto channelsData = content->lifetime().make_state<
		base::flat_set<not_null<History*>>>();

	const auto preview = content->add(object_ptr<FilterChatsPreview>(
		content,
		Data::ChatFilter::Flag(),
		*channelsData));

	const auto refreshPreview = [=] {
		preview->updateData(Data::ChatFilter::Flag(), *channelsData);
	};

	const auto updateChannelsFromSettings = [=] {
		auto &st = AhiGram::Storage::Settings::Instance().data();
		const auto usernames = st.proxyChannels.current();

		channelsData->clear();

		for (const auto &username : usernames) {
			const auto cleanUsername = username.startsWith(u'@')
				? username.mid(1)
				: username;

			if (const auto peer = session->data().peerByUsername(cleanUsername)) {
				if (const auto channel = peer->asChannel()) {
					if (!channel->username().isEmpty()) {
						channelsData->insert(session->data().history(channel));
					}
				}
			} else {
				session->api().request(MTPcontacts_ResolveUsername(
					MTP_flags(0),
					MTP_string(cleanUsername),
					MTP_string()
				)).done(crl::guard(content, [=](const MTPcontacts_ResolvedPeer &result) {
					const auto &data = result.c_contacts_resolvedPeer();
					session->data().processUsers(data.vusers());
					session->data().processChats(data.vchats());

					if (const auto peer = session->data().peerByUsername(cleanUsername)) {
						if (const auto channel = peer->asChannel()) {
							if (!channel->username().isEmpty()) {
								channelsData->insert(
									session->data().history(channel));
								refreshPreview();
							}
						}
					}
				})).send();
			}
		}

		refreshPreview();
	};

	updateChannelsFromSettings();

	preview->peerRemoved(
	) | rpl::on_next([=](not_null<History*> history) {
		const auto channel = history->peer->asChannel();
		if (!channel) {
			return;
		}
		auto rawUsername = channel->username();
		if (rawUsername.startsWith(u'@')) {
			rawUsername = rawUsername.mid(1);
		}

		auto &st = AhiGram::Storage::Settings::Instance().data();
		auto list = st.proxyChannels.current();
		list.removeAll(rawUsername);
		list.removeAll(u'@' + rawUsername);
		st.proxyChannels.force_assign(list);

		channelsData->remove(history);
	}, preview->lifetime());

	const auto addButton = Settings::AddButtonWithIcon(
		content,
		AhiGram::trReactive(u"ahigram_proxy_channels_add"_q),
		st::settingsButton,
		{ &st::settingsIconAdd });

	addButton->setClickedCallback([=] {
		auto controller = std::make_unique<PublicChannelsController>(
			session,
			*channelsData);

		auto initBox = [=](not_null<PeerListBox*> box) {
			box->setCloseByOutsideClick(false);
			box->addButton(tr::lng_settings_save(), crl::guard(box, [=] {
				auto &st = AhiGram::Storage::Settings::Instance().data();
				const auto selected = box->collectSelectedRows();

				QStringList usernames;
				for (const auto &peer : selected) {
					if (const auto channel = peer->asChannel()) {
						auto u = channel->username();
						if (u.startsWith(u'@')) {
							u = u.mid(1);
						}
						if (!u.isEmpty() && !usernames.contains(u)) {
							usernames.append(u);
						}
					}
				}
				st.proxyChannels = usernames;

				channelsData->clear();
				for (const auto &peer : selected) {
					if (const auto channel = peer->asChannel()) {
						if (!channel->username().isEmpty()) {
							channelsData->insert(
								session->data().history(channel));
						}
					}
				}
				refreshPreview();
				box->closeBox();
			}));
			box->addButton(tr::lng_cancel(), [=] { box->closeBox(); });
		};

		_controller->show(
			Box<PeerListBox>(std::move(controller), std::move(initBox)));
	});

	Ui::ResizeFitChild(this, content);
}

} // namespace Settings
