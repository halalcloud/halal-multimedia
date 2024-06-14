# halal-multimedia

  

```
halal-multimedia 是 2dland.cn 线上使用的转码服务（生产级别）

其依赖于FFMPEG和SLS
核心思想是通过预转码将各种原始文件转换到中间层，在需要分发的时候再向用户进行二次stream copy 分发

初始版源自于高升公司(gosun)的分发系统，因此协议也兼容gosun的二进制协议

```

  > IDE：codeblocks

### 项目:

  

```

1.transcode（转码）2.mediaservice（流媒体服务器）

```

  

> 目录结构：

  
  

```

src/ //源码目录

src/*.workspace //workspace

src/bin //二进制目录

src/inc //头文件目录

src/src //公共实现目录

src/*/ //所有模块目录

ThirdParty/ //第三方库

```

  

> 转码：

  

* 流媒体服务器：

服务程序：/src/bin/Debug/server

服务程序配置：/src/bin/Debug/config.json

服务程序执行：./server config.json

  


config.json

  
```json

{
    "root": "/root/media"
}

```

  

```

测试环境运行根目录：\\192.168.10.139\root\media

目录结构

/vhost //本地配置目录

/demo.com/live/ //举例：域名为demo.com路径为/live 流的配置目录

  

```

  

> 配置文件名格式：

  

```*.格式.类型.动作 如 *.gosun.live.push 代表 gosun协议直播推流配置```

> 协议类型：

1.  私有协议格式

2. hls

3. flv

  

类型：

1. live 直播

2. vod 点播

3. file 文件

  

动作：

1. push 推流 或者是上传

2. pull 拉流 或者是下载

  

所有配置文件格式全部统一,列举

> 推流配置

```javascript

{

	"input": {

		"url_format": "%(stream)", //推流变量

		"option": {}

	},

	"output": [

		{

			"url_format": "publish://%(stream)", //内部协议分发
			"media": {
				"id": "gosun"
			},
			"option": {

				"buffer": 1

			}

		},

		{

			"url_format": "publish://%(stream)", //HLS协议分发

			"media": {

				"id": "m3u8"

			},

			"option": {

				"buffer": 3

			}

		},

		{

			"url_format": "publish://%(stream)", //RTMP,HDL协议分发

			"media": {

				"id": "flv"

			},

			"option": {

				"buffer": "1s"

			}

		}

	]

}

//采集转码分发

{

	"input": {

		"url_format": "capture://localhost", //本地采集

		"option": {

			"video": "/dev/video0", //视频采集设备

			"audio": "hw:1,0" //音频采集设备

		}

	},

	"output": [

		{

			"stream": [ //对应转码模板

				0,

				1

			],

			"url_format": "publish://%(stream)", //HLS协议分发

			"media": {

				"id": "m3u8",

				"option": {

					"segment": "3s", //单片时长3秒

					"buffer": 3 //片数量3片

				}

			}

		}

	],

	"template": { //转码模板,与转码配置一致参考转码配置

		"stream": [

			{

				"media": {

					"id": "h264",

					"option": {

						"bitrate": "2m",

						"fps": 25,

						"height": 576,

						"ratioX": 16,

						"ratioY": 9,

						"width": 1024

					}

				},

				"transform": [

					{

						"name": "CIntelVideoEncoder",

						"option": {

							"preset": 1,

							"gop_size": 25

						}

					}

				]

			},

			{

				"media": {

					"id": "aac",

					"option": {

						"bitrate": "64k",

						"channels": 2,

						"sample_rate": 44100

					}

				}

			}

		]

	}

}

```