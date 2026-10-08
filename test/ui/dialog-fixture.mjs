const escape = text => text.replaceAll("&", "&amp;").replaceAll('"', "&quot;").replaceAll("<", "&lt;").replaceAll(">", "&gt;");

export function questionHtml({ kind = "confirm", choices = ["No", "Yes"], defaultChoice = "1", command = "cmd_1_7" } = {}) {
  const forms = choices.map((caption, index) => {
    const fields = { page: "page_1", revision: "9007199254740993", command: `dialog_1_${index}`, csrf: "csrf_1",
      operation: "action", control: `$agiru.answer_dialog_1_${index}` };
    const inputs = Object.entries(fields).map(([name, value]) => `<input type="hidden" name="${name}" value="${value}">`).join("");
    return `<section data-control="${fields.control}" data-kind="action"><form method="post" action="/answers" hx-post="/answers" hx-target="closest article" hx-swap="outerHTML">${inputs}<button type="submit">${escape(caption)}</button></form></section>`;
  }).join("");
  return `<article data-agiru-profile="3" data-view="interaction" data-page="50400" data-handle="page_1" data-revision="9007199254740993" data-state="${kind}" data-call="call_1" data-origin-command="${command}" data-dialog="dialog_1" data-default="${defaultChoice}"><h1>Question</h1><p data-message="message_1">Before &lt;script&gt; 東京.</p><p data-prompt="true">Choose explicitly &lt;script&gt;?</p>${forms}<output data-unsupported-count="0"></output></article>`;
}
