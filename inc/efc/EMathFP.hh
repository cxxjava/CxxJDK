#ifndef __EMathFP_H__
#define __EMathFP_H__

#include "EObject.hh"

namespace efc {

//data type of MathFP int
typedef int intFP;

class EMathFP : public EObject
{
public:

static const intFP PI;// = 12868;
static const intFP E;// = 11134;
static const intFP MAX_VALUE;// = 0x7fffffff;
static const intFP MIN_VALUE;// = 0x80000001;
static const intFP ZERO;// = 0;

static const intFP ONE;// = (1 << 12);
static const intFP HALF;// = (EMathFP::ONE >> 1);
static const intFP TWO;// = (EMathFP::ONE << 1);

static intFP toFP(int i);
static intFP toFP(int i, int f);
static intFP toFP(const char *s);

static int toInt(intFP i);

static intFP add(intFP i, intFP j); //'+'
static intFP sub(intFP i, intFP j); //'-'
static intFP mul(intFP i, intFP j); //'*'
static intFP div(intFP i, intFP j); //'/'

static intFP max(intFP i, intFP j); //max
static intFP min(intFP i, intFP j); //min

static intFP abs(intFP i); //取绝对值
static intFP round(intFP i, int j=0); //对一个实数进行四舍五入，j表示保留小数位数
static intFP trunc(intFP i); //截取一个实数的整数部分
static intFP floor(intFP i); //计算小于指定数的最大整数
static intFP ceil(intFP i); //计算大于指定数的最小整数
static intFP frac(intFP i); //返回一个实数的小数部分，返回值放大了1000倍

static intFP sin(intFP i); //正弦计算，单位是弧度
static intFP cos(intFP i); //余弦计算，单位是弧度
static intFP cot(intFP i); //余切计算，单位是弧度
static intFP tan(intFP i); //正切计算，单位是弧度
static intFP asin(intFP e); //取得反正弦值，单位是弧度
static intFP acos(intFP e); //取得反余弦值，单位是弧度

static intFP exp(intFP i); //自然对数e的次方值
static intFP log(intFP i); //对数，默认底数为10
static intFP pow(intFP i, int j=2); //幂运算
static intFP sqrt(intFP i); //开方根

static intFP toDeg(intFP a); //弧度->角度
static intFP toRad(intFP a); //角度->弧度

private:
static int pow_int(int i, int j);
static int sqrt_byte(int i, int j);
};

} /* namespace efc */
#endif //!__EMathFP_H__
