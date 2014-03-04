; Nightshade installer

[Setup]
DisableStartupPrompt=yes
WizardSmallImageFile=data\icon.bmp
WizardImageFile=data\splash.bmp
WizardImageStretch=no
WizardImageBackColor=clBlack
AppName=Nightshade
AppVerName=Nightshade 11.6.1
DefaultDirName={pf}\Nightshade
DefaultGroupName=Nightshade
UninstallDisplayIcon={app}\nightshade.exe
LicenseFile=COPYING
Compression=zip/9

[Files]
Source: "src\.libs\nightshade.exe"; DestDir: "{app}"
Source: "nscontrol\src\.libs\libnscontrol-*.dll"; DestDir: "{app}"
Source: "README"; DestDir: "{app}"; Flags: isreadme; DestName: "README.rtf"
Source: "INSTALL"; DestDir: "{app}"; DestName: "INSTALL.rtf"
Source: "COPYING"; DestDir: "{app}"; DestName: "GPL.rtf"
Source: "TRADEMARKS"; DestDir: "{app}"; DestName: "TRADEMARKS.rtf"
Source: "AUTHORS"; DestDir: "{app}"; DestName: "AUTHORS.rtf"
;Source: "HACKING"; DestDir: "{app}"; DestName: "HACKING"
Source: "ChangeLog"; DestDir: "{app}";
Source: "win32\bdist\etc\*"; DestDir: "{app}\etc"; Flags: skipifsourcedoesntexist ignoreversion recursesubdirs createallsubdirs
Source: "win32\bdist\lib\*"; DestDir: "{app}\lib"; Flags: skipifsourcedoesntexist ignoreversion recursesubdirs createallsubdirs
Source: "win32\bdist\*.dll"; DestDir: "{app}";
Source: "data\*"; DestDir: "{app}\data"; Excludes: "Makefile*, hipparcos.fab"
Source: "stars\default\*"; DestDir: "{app}\stars\default"; Excludes: "Makefile*"
Source: "data\sky_cultures\*"; DestDir: "{app}\data\sky_cultures"; Flags: recursesubdirs; Excludes: "Makefile*"
Source: "data\scripts\*"; DestDir: "{app}\data\scripts\"; Excludes: "Makefile*, test*"
Source: "doc\*.pdf"; DestDir: "{app}\doc\"; Flags: skipifsourcedoesntexist ignoreversion
;Source: "data\scripts\jupiter-intro\*"; DestDir: "{app}\data\scripts\jupiter-intro"
;Source: "data\scripts\progress-circle\*"; DestDir: "{app}\data\scripts\progress-circle"
; Locales
Source: "po\af.gmo"; DestDir: "{app}\data\locale\af\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\ar.gmo"; DestDir: "{app}\data\locale\ar\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\az.gmo"; DestDir: "{app}\data\locale\az\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\be.gmo"; DestDir: "{app}\data\locale\be\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\bg.gmo"; DestDir: "{app}\data\locale\bg\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\bn.gmo"; DestDir: "{app}\data\locale\bn\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\br.gmo"; DestDir: "{app}\data\locale\br\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\ca.gmo"; DestDir: "{app}\data\locale\ca\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\cs.gmo"; DestDir: "{app}\data\locale\cs\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\cy.gmo"; DestDir: "{app}\data\locale\cy\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\da.gmo"; DestDir: "{app}\data\locale\da\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\de.gmo"; DestDir: "{app}\data\locale\de\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\dv.gmo"; DestDir: "{app}\data\locale\dv\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\el.gmo"; DestDir: "{app}\data\locale\el\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\en.gmo"; DestDir: "{app}\data\locale\en\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\en_AU.gmo"; DestDir: "{app}\data\locale\en_AU\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\en_GB.gmo"; DestDir: "{app}\data\locale\en_GB\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\eo.gmo"; DestDir: "{app}\data\locale\eo\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\es.gmo"; DestDir: "{app}\data\locale\es\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\et.gmo"; DestDir: "{app}\data\locale\et\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\eu.gmo"; DestDir: "{app}\data\locale\eu\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\fa.gmo"; DestDir: "{app}\data\locale\fa\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\fi.gmo"; DestDir: "{app}\data\locale\fi\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\fil.gmo"; DestDir: "{app}\data\locale\fil\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\fr.gmo"; DestDir: "{app}\data\locale\fr\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\ga.gmo"; DestDir: "{app}\data\locale\ga\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\gl.gmo"; DestDir: "{app}\data\locale\gl\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\gu.gmo"; DestDir: "{app}\data\locale\gu\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\haw.gmo"; DestDir: "{app}\data\locale\haw\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\he.gmo"; DestDir: "{app}\data\locale\he\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\hr.gmo"; DestDir: "{app}\data\locale\hr\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\hu.gmo"; DestDir: "{app}\data\locale\hu\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\hy.gmo"; DestDir: "{app}\data\locale\hy\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\id.gmo"; DestDir: "{app}\data\locale\id\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\is.gmo"; DestDir: "{app}\data\locale\is\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\it.gmo"; DestDir: "{app}\data\locale\it\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\ja.gmo"; DestDir: "{app}\data\locale\ja\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\ka.gmo"; DestDir: "{app}\data\locale\ka\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\kn.gmo"; DestDir: "{app}\data\locale\kn\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\ko.gmo"; DestDir: "{app}\data\locale\ko\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\lt.gmo"; DestDir: "{app}\data\locale\lt\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\lv.gmo"; DestDir: "{app}\data\locale\lv\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\mk.gmo"; DestDir: "{app}\data\locale\mk\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\ml.gmo"; DestDir: "{app}\data\locale\ml\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\ms.gmo"; DestDir: "{app}\data\locale\ms\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\mt.gmo"; DestDir: "{app}\data\locale\mt\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\nb.gmo"; DestDir: "{app}\data\locale\nb\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\nl.gmo"; DestDir: "{app}\data\locale\nl\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\nn.gmo"; DestDir: "{app}\data\locale\nn\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\pl.gmo"; DestDir: "{app}\data\locale\pl\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\pt.gmo"; DestDir: "{app}\data\locale\pt\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\pt_BR.gmo"; DestDir: "{app}\data\locale\pt_BR\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\ro.gmo"; DestDir: "{app}\data\locale\ro\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\ru.gmo"; DestDir: "{app}\data\locale\ru\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\sk.gmo"; DestDir: "{app}\data\locale\sk\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\sl.gmo"; DestDir: "{app}\data\locale\sl\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\sr.gmo"; DestDir: "{app}\data\locale\sr\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\sv.gmo"; DestDir: "{app}\data\locale\sv\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\te.gmo"; DestDir: "{app}\data\locale\te\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\th.gmo"; DestDir: "{app}\data\locale\th\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\tl.gmo"; DestDir: "{app}\data\locale\tl\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\tr.gmo"; DestDir: "{app}\data\locale\tr\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\uk.gmo"; DestDir: "{app}\data\locale\uk\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\vi.gmo"; DestDir: "{app}\data\locale\vi\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\zh_CN.gmo"; DestDir: "{app}\data\locale\zh_CN\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\zh_HK.gmo"; DestDir: "{app}\data\locale\zh_HK\LC_MESSAGES\";  DestName: "nightshade.mo"
Source: "po\zh_TW.gmo"; DestDir: "{app}\data\locale\zh_TW\LC_MESSAGES\";  DestName: "nightshade.mo"

Source: "data\default_config.ini"; DestDir: "{app}\config\"
Source: "textures\*"; DestDir: "{app}\textures"; Flags: skipifsourcedoesntexist ignoreversion recursesubdirs createallsubdirs; Excludes: "Makefile*"

[UninstallDelete]
Type: files; Name: "{app}\config\config.ini"
Type: files; Name: "{app}\stdout.txt"
Type: files; Name: "{app}\stderr.txt"

[Icons]
Name: "{group}\nightshade"; Filename: "{app}\nightshade.exe"; WorkingDir: "{app}"; IconFilename: "{app}\data\nightshade.ico"
Name: "{group}\Uninstall nightshade"; Filename: "{uninstallexe}"
Name: "{group}\config.ini"; Filename: "{localappdata}\nightshade\config.ini"
Name: "{group}\Documentation"; Filename: "{app}\doc\"

