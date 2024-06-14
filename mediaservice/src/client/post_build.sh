if [ -d "../bin/Release/transcode_dump" ]; then
rm -r ../bin/Release/transcode_dump
fi

if [ -d "../bin/Release/server_dump" ]; then
rm -r ../bin/Release/server_dump
fi

if [ -d "../bin/Release/app" ]; then
rm -r ../bin/Release/app
fi

if [ ! -d "../../../publish" ]; then
mkdir ../../../publish
fi

DAT=$(date +%Y-%m-%d_%H-%M-%S)
tar czvf "../../../publish/transcode_"$DAT".tar.gz" ../bin/Release/
