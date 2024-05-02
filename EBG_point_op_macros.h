#pragma once

/*
point<T>: 2-sized vector (vec2<T>)

P stants for: point
I: built-in type
_: P or I (both can usable as type [always second type])
d: diffrent types for 'I'

T, Tn: Type (Type specified macro)
t: template
i: intager and unsigned types only

A: Arithmetic
L: boolean logic
E: equal (+=, -=)
OP: Operator

S: symbol (operation [+, -, *, <<, vb.])
J: junction, joint (&&, ||)

number on last: size of vector (2: point)

TPPAOP: Type specified Point & Point input arithmetic operator
*/

#define TP_AOP2(S, T1, T2, X, Y)							\
constexpr point<T1> operator S(point<T1> a, T2 b) {			\
	return point<T1>(a.x S X, a.y S Y);						\
}

#define TPPAOP2(S, T) TP_AOP2(S, T, point<T>, b.x, b.y)
#define TPIAOP2(S, T) TP_AOP2(S, T, T, b, b)
#define TPIdAOP2(S, T1, T2) TP_AOP2(S, T1, T2, b, b)

#define tPPAOP2(S) template <typename T> TPPAOP2(S, T)
#define iPPAOP2(S) TPPAOP2(S, int) TPPAOP2(S, unsigned)

#define tPIAOP2(S) template <typename T> TPIAOP2(S, T)
#define iPIAOP2(S) TPIAOP2(S, int) TPIAOP2(S, unsigned)
#define iPIdAOP2(S) TPIdAOP2(S, int, unsigned) TPIdAOP2(S, unsigned, int)

#define tP_AOP2(S) tPPAOP2(S) tPIAOP2(S)

#define tPPLOP2(S, J)										\
template <typename T>										\
constexpr bool operator S(point<T> a, point<T> b) {			\
	return (a.x S b.x) J (a.y S b.y);						\
}

#define tPPEAOP2(S)											\
template <typename T>										\
void operator S=(point<T>& a, point<T> b) {					\
	a.x S= b.x; a.y S= b.y;									\
}

#define TPIEAOP2(S, T)										\
void operator S=(point<T>& a, T b) {						\
	a.x S= b; a.y S= b;										\
}

#define tPIEAOP2(S) template <typename T> TPIEAOP2(S, T)
#define iPIEAOP2(S) TPIEAOP2(S, int) TPIEAOP2(S, unsigned)

#define ALL_OPS2											\
tP_AOP2(+) tP_AOP2(-) tP_AOP2(*) tP_AOP2(/)					\
iPPAOP2(%) iPPAOP2(&) iPPAOP2(<<) iPPAOP2(>>)				\
iPIAOP2(%) iPIAOP2(&) iPIAOP2(|) iPIAOP2(<<) iPIAOP2(>>)	\
iPIdAOP2(<<) iPIdAOP2(>>) iPIdAOP2(+) iPIdAOP2(-)			\
tPPLOP2(==, &&) tPPLOP2(!=, ||) tPPLOP2(<, &&)				\
tPPLOP2(>, &&) tPPLOP2(<=, &&) tPPLOP2(>=, &&)				\
tPPEAOP2(+) tPPEAOP2(-)										\
tPIEAOP2(+) tPIEAOP2(-) tPIEAOP2(*) tPIEAOP2(/)				\
iPIEAOP2(%) iPIEAOP2(&) iPIEAOP2(<<) iPIEAOP2(>>)

// vector3

#define TP_AOP3(S, T1, T2, X, Y, Z)							\
constexpr vec3<T1> operator S(vec3<T1> a, T2 b) {			\
	return vec3<T1>(a.x S X, a.y S Y, a.z S Z);				\
}

#define TPPAOP3(S, T) TP_AOP3(S, T, vec3<T>, b.x, b.y, b.z)
#define TPIAOP3(S, T) TP_AOP3(S, T, T, b, b, b)

#define tPPAOP3(S) template <typename T> TPPAOP3(S, T)
#define iPPAOP3(S) TPPAOP3(S, int) TPPAOP3(S, unsigned)

#define tPIAOP3(S) template <typename T> TPIAOP3(S, T)
#define iPIAOP3(S) TPIAOP3(S, int) TPIAOP3(S, unsigned)

#define tP_AOP3(S) tPPAOP3(S) tPIAOP3(S)

#define tPPLOP3(S, J)										\
template <typename T>										\
constexpr bool operator S(vec3<T> a, vec3<T> b) {			\
	return (a.x S b.x) J (a.y S b.y) J (a.z S b.z);			\
}

#define tPPEAOP3(S)											\
template <typename T>										\
void operator S=(vec3<T>& a, vec3<T> b) {					\
	a.x S= b.x; a.y S= b.y; a.z S= b.z;						\
}

#define TPIEAOP3(S, T)										\
void operator S=(vec3<T>& a, T b) {							\
	a.x S= b; a.y S= b; a.z S= b;							\
}

#define tPIEAOP3(S) template <typename T> TPIEAOP3(S, T)
#define iPIEAOP3(S) TPIEAOP3(S, int) TPIEAOP3(S, unsigned)

#define ALL_OPS3											\
tP_AOP3(+) tP_AOP3(-) tP_AOP3(*) tP_AOP3(/)					\
iPPAOP3(%) iPPAOP3(&) iPPAOP3(<<) iPPAOP3(>>)				\
iPIAOP3(%) iPIAOP3(&) iPIAOP3(|) iPIAOP3(<<) iPIAOP3(>>)	\
tPPLOP3(==, &&) tPPLOP3(!=, ||) tPPLOP3(<, &&)				\
tPPLOP3(>, &&) tPPLOP3(<=, &&) tPPLOP3(>=, &&)				\
tPPEAOP3(+) tPPEAOP3(-)										\
tPIEAOP3(+) tPIEAOP3(-) tPIEAOP3(*) tPIEAOP3(/)				\
iPIEAOP3(%) iPIEAOP3(&) iPIEAOP3(<<) iPIEAOP3(>>)