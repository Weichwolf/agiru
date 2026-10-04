# Dependency notices

- `encoding-data.txt`: original .NET Foundation MIT notice and Unicode data notice
  for the Windows-1252 and ISO-8859-1 mapping/best-fit data in `src/net/CodePage.cpp`.
- Authority: original BC29 `NavBase64Converter` with CLR 10.0.12's
  `System.Text.Encoding.CodePages`; data SHA256
  `99e05c67832a638543b851ab05a5538ca3889ecbfe397e86a1e29371de824815` (0035).
- ISO-8859-1 reference SHA256:
  `13e35a97cdfdb24006ba3a631d330ec6791037ffa838cb477f0a3767d622a64d`.
- ASCII reference SHA256:
  `5ce26fd8b4b86f18aedac016ba0a6de4f8d1acd74fee5e3c69b8bee3baa46278`.
- The converter is agiru-owned C++; there is no .NET or ICU runtime dependency.
- Retain this directory with source and binary distributions. These notices do not
  relicense third-party data as agiru-owned code.
