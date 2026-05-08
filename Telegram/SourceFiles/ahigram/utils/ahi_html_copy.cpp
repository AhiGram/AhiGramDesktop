/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahigram/utils/ahi_html_copy.h"

#include "ui/text/text_entity.h"
#include "ui/widgets/fields/input_field.h"

#include <QtCore/QMimeData>
#include <QtCore/QString>

namespace AhiGram {
namespace {

const auto kMarkdownBoldPrefix = u"**\\"_q;
const auto kHtmlGtPrefix = u"&gt;\\"_q;
const auto kGtPrefix = u">\\"_q;

[[nodiscard]] QString CleanupBlockquotes(QString html) {
	html.replace(u"</blockquote><blockquote>"_q, u""_q);
	html.replace(u"</blockquote> <blockquote>"_q, u" "_q);
	html.replace(u"</blockquote>\n<blockquote>"_q, u"\n"_q);
	html.replace(u"<blockquote> </blockquote>"_q, u" "_q);
	html.replace(u"<blockquote></blockquote>"_q, u""_q);
	html.replace(u"<blockquote> &gt; </blockquote>"_q, u" > "_q);
	html.replace(u"</blockquote><a"_q, u"<a"_q);
	html.replace(u"</a><blockquote>"_q, u"</a>"_q);
	return html;
}

[[nodiscard]] QString ConvertTextWithTagsToHtml(
		const QString &text,
		const TextWithTags::Tags &tags) {
	if (text.isEmpty()) {
		return QString();
	}

	struct TagPosition {
		int offset;
		QString tag;
		bool opening;
	};

	auto positions = std::vector<TagPosition>();
	positions.reserve(tags.size() * 2);

	for (const auto &tag : tags) {
		const auto id = tag.id;
		QString openTag;
		QString closeTag;

		if (id == Ui::InputField::kTagBold) {
			openTag = u"<b>"_q;
			closeTag = u"</b>"_q;
		} else if (id == Ui::InputField::kTagItalic) {
			openTag = u"<i>"_q;
			closeTag = u"</i>"_q;
		} else if (id == Ui::InputField::kTagUnderline) {
			openTag = u"<u>"_q;
			closeTag = u"</u>"_q;
		} else if (id == Ui::InputField::kTagStrikeOut) {
			openTag = u"<s>"_q;
			closeTag = u"</s>"_q;
		} else if (id == Ui::InputField::kTagCode) {
			openTag = u"<code>"_q;
			closeTag = u"</code>"_q;
		} else if (id.startsWith(Ui::InputField::kTagPre)) {
			const auto language = id.mid(Ui::InputField::kTagPre.size());
			if (language.isEmpty()) {
				openTag = u"<pre>"_q;
			} else {
				openTag = u"<pre data-language=\""_q + language.toHtmlEscaped() + u"\">"_q;
			}
			closeTag = u"</pre>"_q;
		} else if (id == Ui::InputField::kTagBlockquote
			|| id == Ui::InputField::kTagBlockquoteCollapsed) {
			openTag = u"<blockquote>"_q;
			closeTag = u"</blockquote>"_q;
		} else if (id == Ui::InputField::kTagSpoiler) {
			openTag = u"<span class=\"spoiler\">"_q;
			closeTag = u"</span>"_q;
		} else if (id.contains(Ui::InputField::kCustomEmojiTagStart)) {
			const auto startPos = id.indexOf(Ui::InputField::kCustomEmojiTagStart);
			auto emojiId = id.mid(startPos + Ui::InputField::kCustomEmojiTagStart.size());
			const auto questionIndex = emojiId.indexOf('?');
			if (questionIndex >= 0) {
				emojiId = emojiId.left(questionIndex);
			}
			openTag = u"<tg-emoji emoji-id=\""_q + emojiId.toHtmlEscaped() + u"\">"_q;
			closeTag = u"</tg-emoji>"_q;
		} else if (Ui::InputField::IsValidMarkdownLink(id)) {
			auto cleanUrl = id;
			if (cleanUrl.startsWith(kMarkdownBoldPrefix)) {
				cleanUrl = cleanUrl.mid(kMarkdownBoldPrefix.size());
			} else if (cleanUrl.startsWith(kHtmlGtPrefix)) {
				cleanUrl = cleanUrl.mid(kHtmlGtPrefix.size());
			} else if (cleanUrl.startsWith(kGtPrefix)) {
				cleanUrl = cleanUrl.mid(kGtPrefix.size());
			}
			openTag = u"<a href=\""_q + cleanUrl.toHtmlEscaped() + u"\">"_q;
			closeTag = u"</a>"_q;
		} else {
			continue;
		}

		positions.push_back({ tag.offset, openTag, true });
		positions.push_back({ tag.offset + tag.length, closeTag, false });
	}

	ranges::sort(positions, [](const TagPosition &a, const TagPosition &b) {
		if (a.offset != b.offset) {
			return a.offset < b.offset;
		}
		return !a.opening && b.opening;
	});

	auto result = QString();
	result.reserve(text.size() * 2);
	auto offset = 0;

	for (const auto &pos : positions) {
		if (pos.offset > offset) {
			result.append(text.mid(offset, pos.offset - offset).toHtmlEscaped());
		}
		result.append(pos.tag);
		offset = pos.offset;
	}

	if (offset < text.size()) {
		result.append(text.mid(offset).toHtmlEscaped());
	}

	return CleanupBlockquotes(result);
}

} // namespace

std::unique_ptr<QMimeData> CopyAsHtml(const TextForMimeData &text) {
	if (text.rich.text.isEmpty()) {
		return nullptr;
	}

	const auto tags = TextUtilities::ConvertEntitiesToTextTags(
		text.rich.entities);
	const auto html = ConvertTextWithTagsToHtml(text.rich.text, tags);

	if (html.isEmpty()) {
		return nullptr;
	}

	auto result = std::make_unique<QMimeData>();
	result->setText(html);
	return result;
}

} // namespace AhiGram
