/**
 * 基础数据结构在这里定义.
 */
#include "base.h"
#include "wzdBits.h"
namespace wzd {

__int64 Get33_1_32(CBits& bit)
{
	__int64 i64 = bit.Read();
	i64 <<= 32;
	i64 += bit.Read(32);
	return i64;
}

__int64 Get33_3_15_15(CBits& bit)
{
	__int64 i64 = bit.Read(3);
	i64 <<= 30;
	bit.Read();
	int iTmp  = bit.Read(15);
	i64 += iTmp <<15;
	bit.Read();
	iTmp = bit.Read(15);
	i64 += iTmp;
	bit.Read();
	return i64;
}
} // namespace wzd
