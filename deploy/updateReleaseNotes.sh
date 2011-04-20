#!/bin/sh

targetdir=$1
version=$2

if [ -z "${targetdir}" ]; then
	echo "Usage: $0 <version> <targetdir>"
	exit 1
fi

if [ ! -d "${targetdir}" ]; then
	echo "${targetdir} does not exists"
	exit 2
fi

function extractXML()
{
	file=$1
	key=$2

	grep -1 ${key} ${file} | tail -1 | cut -f2 -d\> | cut -f1 -d\<
}

set -e

pa_version=$(extractXML ${targetdir}/Library/Frameworks/pulse.framework/Resources/Info.plist CFBundleShortVersionString)
pulseconsle_version=$(extractXML ${targetdir}/Applications/PulseConsole.app/Contents/Info.plist CFBundleVersion)
halplugin_version=$(extractXML ${targetdir}/Library/Audio/Plug-Ins/HAL/PulseAudio.plugin/Contents/Info.plist CFBundleShortVersionString)
prefpane_version=$(extractXML ${targetdir}/Library/PreferencePanes/PulseAudio.prefPane/Contents/Info.plist CFBundleVersion)

cat > ${targetdir}/rn <<EOF__
Version ${version}

  This package version contains the following components
    - PulseAudio daemon and tools:  ${pa_version}
    - HAL plugin:                   ${halplugin_version}
    - Preference Pane:              ${prefpane_version}
    - PulseConsole:                 ${pulseconsle_version}

EOF__

cat ReleaseNotes.txt >> ${targetdir}/rn
cat ${targetdir}/rn > ReleaseNotes.txt
rm -f ${targetdir}/rn

