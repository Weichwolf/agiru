Type:     task
Status:   open
Parent:   0065
Area:     rt, gen
Source:   developer/triggers-auto/xmlport/devenv-oninitxmlport-xmlport-trigger.md, developer/triggers-auto/xmlport/devenv-onprexmlport-xmlport-trigger.md, developer/triggers-auto/xmlport/devenv-onpostxmlport-xmlport-trigger.md
Verdict:  fehlt
Class:    activation

# An XmlPort's three object triggers bracket the whole transfer

`OnInitXmlPort`, `OnPreXmlPort` and `OnPostXmlPort` are the XmlPort's analogue of a report's
`OnInitReport` / `OnPreReport` / `OnPostReport` (0302, 0303, 0304): initialise, then -- after the
request page, if there is one -- prepare, then clean up when every element has been processed.

**They are one task**, because they are three points in one driver loop and an implementation that
places one places all three.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

The three together: **412 declarations**, against 457 `.XmlPort.al` files -- so most XmlPorts use at
least one.

## The IST-state

XmlPort has no generator: board:0034's object-kind table lists it among the kinds with none, and
board:0065 is the item.

## The choice

The XmlPort driver calls them at the three points, with the request page (0301) between the first
two -- `devenv-request-pages.md` documents that "a request page is a page that is run before the
XMLport starts to execute", the same shape a report has.

**`OnPostXmlPort` runs only on completion**, like `OnPostReport` (0304), and for the same reason: it
closes streams and removes temporary files that a failed run never made.

## Ordering

Blocked on board:0065.

## Gate, and its negative control

An XmlPort whose `OnPreXmlPort` quits: no element is processed and `OnPostXmlPort` does not run.

**The negative control is `OnPostXmlPort`** -- a driver that runs it unconditionally closes streams
a failed run never opened.
