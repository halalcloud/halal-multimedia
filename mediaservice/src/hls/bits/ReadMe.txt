从FFMPEG中提取出来的位操作函数库．
compat目录对应FFMPEG源码中的同名目录，同名文件。
libavutil中的文件对应FFMPEG源码中的同名文件
put_bits.h/get_bits.h对应libavcodec目录中的同名文件。修改如下：
1. 把函数声明的static inline 注释了
2. 把Get/PUtContext提取到bbits.h中了。并在bbits.h中声明了函数原型
3. 由于不明原因，找不到av_log函数实现.在get_bits.h中加入了一个空实现。
golomb.c 对应libavcodec目录中的同名文件
golomb_.cpp对应libavcodec目录中的glomb.h。修改如下：
1. 引入bbits.h libavutil/intmath.h
2. 把函数声明的static inline 注释了．并在bbits.h中声明了函数原型。
3. 之所以不叫golomb.cpp是由于它和golomb.c生成的文件都叫golomb.o导致函数被覆盖。
当需要跨平台能力的时候（默认是linux），需要替换config.h文件（它可以从windows中的FFMPEG中得到）。

