// SPDX-License-Identifier: GPL-2.0+
/*
 * Test cases csum_partial, csum_fold, ip_fast_csum, csum_ipv6_magic
 */

#include <kunit/test.h>
#include <asm/checksum.h>
#include <net/ip6_checksum.h>

#define MAX_LEN 512
#define MAX_ALIGN 64
#define TEST_BUFLEN (MAX_LEN + MAX_ALIGN)

#define IPv4_MIN_WORDS 5
#define IPv4_MAX_WORDS 15
#define NUM_IPv6_TESTS 200
#define NUM_IP_FAST_CSUM_TESTS 181

/* Values for a little endian CPU. Byte swap each half on big endian CPU. */
static const u32 random_init_sum = 0x2847aab;
static const u8 random_buf[] = {
	0xac, 0xd7, 0x76, 0x69, 0x6e, 0xf2, 0x93, 0x2c, 0x1f, 0xe0, 0xde, 0x86,
	0x8f, 0x54, 0x33, 0x90, 0x95, 0xbf, 0xff, 0xb9, 0xea, 0x62, 0x6e, 0xb5,
	0xd3, 0x4f, 0xf5, 0x60, 0x50, 0x5c, 0xc7, 0xfa, 0x6d, 0x1a, 0xc7, 0xf0,
	0xd2, 0x2c, 0x12, 0x3d, 0x88, 0xe3, 0x14, 0x21, 0xb1, 0x5e, 0x45, 0x31,
	0xa2, 0x85, 0x36, 0x76, 0xba, 0xd8, 0xad, 0xbb, 0x9e, 0x49, 0x8f, 0xf7,
	0xce, 0xea, 0xef, 0xca, 0x2c, 0x29, 0xf7, 0x15, 0x5c, 0x1d, 0x4d, 0x09,
	0x1f, 0xe2, 0x14, 0x31, 0x8c, 0x07, 0x57, 0x23, 0x1f, 0x6f, 0x03, 0xe1,
	0x93, 0x19, 0x53, 0x03, 0x45, 0x49, 0x9a, 0x3b, 0x8e, 0x0c, 0x12, 0x5d,
	0x8a, 0xb8, 0x9b, 0x8c, 0x9a, 0x03, 0xe5, 0xa2, 0x43, 0xd2, 0x3b, 0x4e,
	0x7e, 0x30, 0x3c, 0x22, 0x2d, 0xc5, 0xfc, 0x9e, 0xdb, 0xc6, 0xf9, 0x69,
	0x12, 0x39, 0x1f, 0xa0, 0x11, 0x0c, 0x3f, 0xf5, 0x53, 0xc9, 0x30, 0xfb,
	0xb0, 0xdd, 0x21, 0x1d, 0x34, 0xe2, 0x65, 0x30, 0xf1, 0xe8, 0x1b, 0xe7,
	0x55, 0x0d, 0xeb, 0xbd, 0xcc, 0x9d, 0x24, 0xa4, 0xad, 0xa7, 0x93, 0x47,
	0x19, 0x2e, 0xc4, 0x5c, 0x3b, 0xc7, 0x6d, 0x95, 0x0c, 0x47, 0x60, 0xaf,
	0x5b, 0x47, 0xee, 0xdc, 0x31, 0x31, 0x14, 0x12, 0x7e, 0x9e, 0x45, 0xb1,
	0xc1, 0x69, 0x4b, 0x84, 0xfc, 0x88, 0xc1, 0x9e, 0x46, 0xb4, 0xc2, 0x25,
	0xc5, 0x6c, 0x4c, 0x22, 0x58, 0x5c, 0xbe, 0xff, 0xea, 0x88, 0x88, 0x7a,
	0xcb, 0x1c, 0x5d, 0x63, 0xa1, 0xf2, 0x33, 0x0c, 0xa2, 0x16, 0x0b, 0x6e,
	0x2b, 0x79, 0x58, 0xf7, 0xac, 0xd3, 0x6a, 0x3f, 0x81, 0x57, 0x48, 0x45,
	0xe3, 0x7c, 0xdc, 0xd6, 0x34, 0x7e, 0xe6, 0x73, 0xfa, 0xcb, 0x31, 0x18,
	0xa9, 0x0b, 0xee, 0x6b, 0x99, 0xb9, 0x2d, 0xde, 0x22, 0x0e, 0x71, 0x57,
	0x0e, 0x9b, 0x11, 0xd1, 0x15, 0x41, 0xd0, 0x6b, 0x50, 0x8a, 0x23, 0x64,
	0xe3, 0x9c, 0xb3, 0x55, 0x09, 0xe9, 0x32, 0x67, 0xf9, 0xe0, 0x73, 0xf1,
	0x60, 0x66, 0x0b, 0x88, 0x79, 0x8d, 0x4b, 0x52, 0x83, 0x20, 0x26, 0x78,
	0x49, 0x27, 0xe7, 0x3e, 0x29, 0xa8, 0x18, 0x82, 0x41, 0xdd, 0x1e, 0xcc,
	0x3b, 0xc4, 0x65, 0xd1, 0x21, 0x40, 0x72, 0xb2, 0x87, 0x5e, 0x16, 0x10,
	0x80, 0x3f, 0x4b, 0x58, 0x1c, 0xc2, 0x79, 0x20, 0xf0, 0xe0, 0x80, 0xd3,
	0x52, 0xa5, 0x19, 0x6e, 0x47, 0x90, 0x08, 0xf5, 0x50, 0xe2, 0xd6, 0xae,
	0xe9, 0x2e, 0xdc, 0xd5, 0xb4, 0x90, 0x1f, 0x79, 0x49, 0x82, 0x21, 0x84,
	0xa0, 0xb5, 0x2f, 0xff, 0x30, 0x71, 0xed, 0x80, 0x68, 0xb1, 0x6d, 0xef,
	0xf6, 0xcf, 0xb8, 0x41, 0x79, 0xf5, 0x01, 0xbc, 0x0c, 0x9b, 0x0e, 0x06,
	0xf3, 0xb0, 0xbb, 0x97, 0xb8, 0xb1, 0xfd, 0x51, 0x4e, 0xef, 0x0a, 0x3d,
	0x7a, 0x3d, 0xbd, 0x61, 0x00, 0xa2, 0xb3, 0xf0, 0x1d, 0x77, 0x7b, 0x6c,
	0x01, 0x61, 0xa5, 0xa3, 0xdb, 0xd5, 0xd5, 0xf4, 0xb5, 0x28, 0x9f, 0x0a,
	0xa3, 0x82, 0x5f, 0x4b, 0x40, 0x0f, 0x05, 0x0e, 0x78, 0xed, 0xbf, 0x17,
	0xf6, 0x5a, 0x8a, 0x7d, 0xf9, 0x45, 0xc1, 0xd7, 0x1b, 0x9d, 0x6c, 0x07,
	0x88, 0xf3, 0xbc, 0xf1, 0xea, 0x28, 0x1f, 0xb8, 0x7a, 0x60, 0x3c, 0xce,
	0x3e, 0x50, 0xb2, 0x0b, 0xcf, 0xe5, 0x08, 0x1f, 0x48, 0x04, 0xf9, 0x35,
	0x29, 0x15, 0xbe, 0x82, 0x96, 0xc2, 0x55, 0x04, 0x6c, 0x19, 0x45, 0x29,
	0x0b, 0xb6, 0x49, 0x12, 0xfb, 0x8d, 0x1b, 0x75, 0x8b, 0xd9, 0x6a, 0x5c,
	0xbe, 0x46, 0x2b, 0x41, 0xfe, 0x21, 0xad, 0x1f, 0x75, 0xe7, 0x90, 0x3d,
	0xe1, 0xdf, 0x4b, 0xe1, 0x81, 0xe2, 0x17, 0x02, 0x7b, 0x58, 0x8b, 0x92,
	0x1a, 0xac, 0x46, 0xdd, 0x2e, 0xce, 0x40, 0x09
};

/* Values for a little endian CPU. Byte swap on big endian CPU. */
static const u16 expected_results[] = {
	0x82d0, 0x8224, 0xab23, 0xaaad, 0x41ad, 0x413f, 0x4f3e, 0x4eab, 0x22ab,
	0x228c, 0x428b, 0x41ad, 0xbbac, 0xbb1d, 0x671d, 0x66ea, 0xd6e9, 0xd654,
	0x1754, 0x1655, 0x5d54, 0x5c6a, 0xfa69, 0xf9fb, 0x44fb, 0x4428, 0xf527,
	0xf432, 0x9432, 0x93e2, 0x37e2, 0x371b, 0x3d1a, 0x3cad, 0x22ad, 0x21e6,
	0x31e5, 0x3113, 0x0513, 0x0501, 0xc800, 0xc778, 0xe477, 0xe463, 0xc363,
	0xc2b2, 0x64b2, 0x646d, 0x336d, 0x32cb, 0xadca, 0xad94, 0x3794, 0x36da,
	0x5ed9, 0x5e2c, 0xa32b, 0xa28d, 0x598d, 0x58fe, 0x61fd, 0x612f, 0x772e,
	0x763f, 0xac3e, 0xac12, 0x8312, 0x821b, 0x6d1b, 0x6cbf, 0x4fbf, 0x4f72,
	0x4672, 0x4653, 0x6452, 0x643e, 0x333e, 0x32b2, 0x2bb2, 0x2b5b, 0x085b,
	0x083c, 0x993b, 0x9938, 0xb837, 0xb7a4, 0x9ea4, 0x9e51, 0x9b51, 0x9b0c,
	0x520c, 0x5172, 0x1672, 0x15e4, 0x09e4, 0x09d2, 0xacd1, 0xac47, 0xf446,
	0xf3ab, 0x67ab, 0x6711, 0x6411, 0x632c, 0xc12b, 0xc0e8, 0xeee7, 0xeeac,
	0xa0ac, 0xa02e, 0x702e, 0x6ff2, 0x4df2, 0x4dc5, 0x88c4, 0x87c8, 0xe9c7,
	0xe8ec, 0x22ec, 0x21f3, 0xb8f2, 0xb8e0, 0x7fe0, 0x7fc1, 0xdfc0, 0xdfaf,
	0xd3af, 0xd370, 0xde6f, 0xde1c, 0x151c, 0x14ec, 0x19eb, 0x193b, 0x3c3a,
	0x3c19, 0x1f19, 0x1ee5, 0x3ce4, 0x3c7f, 0x0c7f, 0x0b8e, 0x238d, 0x2372,
	0x3c71, 0x3c1c, 0x2f1c, 0x2e31, 0x7130, 0x7064, 0xd363, 0xd33f, 0x2f3f,
	0x2e92, 0x8791, 0x86fe, 0x3ffe, 0x3fe5, 0x11e5, 0x1121, 0xb520, 0xb4e5,
	0xede4, 0xed77, 0x5877, 0x586b, 0x116b, 0x110b, 0x620a, 0x61af, 0x1aaf,
	0x19c1, 0x3dc0, 0x3d8f, 0x0c8f, 0x0c7b, 0xfa7a, 0xf9fc, 0x5bfc, 0x5bb7,
	0xaab6, 0xa9f5, 0x40f5, 0x40aa, 0xbca9, 0xbbad, 0x33ad, 0x32ec, 0x94eb,
	0x94a5, 0xe0a4, 0xdfe2, 0xbae2, 0xba1d, 0x4e1d, 0x4dd1, 0x2bd1, 0x2b79,
	0xcf78, 0xceba, 0xcfb9, 0xcecf, 0x46cf, 0x4647, 0xcc46, 0xcb7b, 0xaf7b,
	0xaf1e, 0x4c1e, 0x4b7d, 0x597c, 0x5949, 0x4d49, 0x4ca7, 0x36a7, 0x369c,
	0xc89b, 0xc870, 0x4f70, 0x4f18, 0x5817, 0x576b, 0x846a, 0x8400, 0x4500,
	0x447f, 0xed7e, 0xed36, 0xa836, 0xa753, 0x2b53, 0x2a77, 0x5476, 0x5442,
	0xd641, 0xd55b, 0x625b, 0x6161, 0x9660, 0x962f, 0x7e2f, 0x7d86, 0x7286,
	0x7198, 0x0698, 0x05ff, 0x4cfe, 0x4cd1, 0x6ed0, 0x6eae, 0x60ae, 0x603d,
	0x093d, 0x092f, 0x6e2e, 0x6e1d, 0x9d1c, 0x9d07, 0x5c07, 0x5b37, 0xf036,
	0xefe6, 0x65e6, 0x65c3, 0x01c3, 0x00e0, 0x64df, 0x642c, 0x0f2c, 0x0f23,
	0x2622, 0x25f0, 0xbeef, 0xbdf6, 0xddf5, 0xdd82, 0xec81, 0xec21, 0x8621,
	0x8616, 0xfe15, 0xfd9c, 0x709c, 0x7051, 0x1e51, 0x1dce, 0xfdcd, 0xfda7,
	0x85a7, 0x855e, 0x5e5e, 0x5d77, 0x1f77, 0x1f4e, 0x774d, 0x7735, 0xf534,
	0xf4f3, 0x17f3, 0x17d5, 0x4bd4, 0x4b99, 0x8798, 0x8733, 0xb632, 0xb611,
	0x7611, 0x759f, 0xc39e, 0xc317, 0x6517, 0x6501, 0x5501, 0x5481, 0x1581,
	0x1536, 0xbd35, 0xbd19, 0xfb18, 0xfa9f, 0xda9f, 0xd9af, 0xf9ae, 0xf92e,
	0x262e, 0x25dc, 0x80db, 0x80c2, 0x12c2, 0x127b, 0x827a, 0x8272, 0x8d71,
	0x8d21, 0xab20, 0xaa4a, 0xfc49, 0xfb60, 0xcd60, 0xcc84, 0xf783, 0xf6cf,
	0x66cf, 0x66b0, 0xedaf, 0xed66, 0x6b66, 0x6b45, 0xe744, 0xe6a4, 0x31a4,
	0x3175, 0x3274, 0x3244, 0xc143, 0xc056, 0x4056, 0x3fee, 0x8eed, 0x8e80,
	0x9f7f, 0x9e89, 0xcf88, 0xced0, 0x8dd0, 0x8d57, 0x9856, 0x9855, 0xdc54,
	0xdc48, 0x4148, 0x413a, 0x3b3a, 0x3a47, 0x8a46, 0x898b, 0xf28a, 0xf1d2,
	0x40d2, 0x3fd5, 0xeed4, 0xee86, 0xff85, 0xff7b, 0xc27b, 0xc201, 0x8501,
	0x8444, 0x2344, 0x2344, 0x8143, 0x8090, 0x908f, 0x9072, 0x1972, 0x18f7,
	0xacf6, 0xacf5, 0x4bf5, 0x4b50, 0xa84f, 0xa774, 0xd273, 0xd19e, 0xdd9d,
	0xdce8, 0xb4e8, 0xb449, 0xaa49, 0xa9a6, 0x27a6, 0x2747, 0xdc46, 0xdc06,
	0xcd06, 0xcd01, 0xbf01, 0xbe89, 0xd188, 0xd0c9, 0xb9c9, 0xb8d3, 0x5ed3,
	0x5e49, 0xe148, 0xe04f, 0x9b4f, 0x9a8e, 0xc38d, 0xc372, 0x2672, 0x2606,
	0x1f06, 0x1e7e, 0x2b7d, 0x2ac1, 0x39c0, 0x38d6, 0x10d6, 0x10b7, 0x58b6,
	0x583c, 0xf83b, 0xf7ff, 0x29ff, 0x29c1, 0xd9c0, 0xd90e, 0xce0e, 0xcd3f,
	0xe83e, 0xe836, 0xc936, 0xc8ee, 0xc4ee, 0xc3f5, 0x8ef5, 0x8ecc, 0x79cc,
	0x790e, 0xf70d, 0xf677, 0x3477, 0x3422, 0x3022, 0x2fb6, 0x16b6, 0x1671,
	0xed70, 0xed65, 0x3765, 0x371c, 0x251c, 0x2421, 0x9720, 0x9705, 0x2205,
	0x217a, 0x4879, 0x480f, 0xec0e, 0xeb50, 0xa550, 0xa525, 0x6425, 0x6327,
	0x4227, 0x417a, 0x227a, 0x2205, 0x3b04, 0x3a74, 0xfd73, 0xfc92, 0x1d92,
	0x1d47, 0x3c46, 0x3bc5, 0x59c4, 0x59ad, 0x57ad, 0x5732, 0xff31, 0xfea6,
	0x6ca6, 0x6c8c, 0xc08b, 0xc045, 0xe344, 0xe316, 0x1516, 0x14d6,
};

/* Values for a little endian CPU. Byte swap each half on big endian CPU. */
static const u32 init_sums_no_overflow[] = {
	0xffffffff, 0xfffffffb, 0xfffffbfb, 0xfffffbf7, 0xfffff7f7, 0xfffff7f3,
	0xfffff3f3, 0xfffff3ef, 0xffffefef, 0xffffefeb, 0xffffebeb, 0xffffebe7,
	0xffffe7e7, 0xffffe7e3, 0xffffe3e3, 0xffffe3df, 0xffffdfdf, 0xffffdfdb,
	0xffffdbdb, 0xffffdbd7, 0xffffd7d7, 0xffffd7d3, 0xffffd3d3, 0xffffd3cf,
	0xffffcfcf, 0xffffcfcb, 0xffffcbcb, 0xffffcbc7, 0xffffc7c7, 0xffffc7c3,
	0xffffc3c3, 0xffffc3bf, 0xffffbfbf, 0xffffbfbb, 0xffffbbbb, 0xffffbbb7,
	0xffffb7b7, 0xffffb7b3, 0xffffb3b3, 0xffffb3af, 0xffffafaf, 0xffffafab,
	0xffffabab, 0xffffaba7, 0xffffa7a7, 0xffffa7a3, 0xffffa3a3, 0xffffa39f,
	0xffff9f9f, 0xffff9f9b, 0xffff9b9b, 0xffff9b97, 0xffff9797, 0xffff9793,
	0xffff9393, 0xffff938f, 0xffff8f8f, 0xffff8f8b, 0xffff8b8b, 0xffff8b87,
	0xffff8787, 0xffff8783, 0xffff8383, 0xffff837f, 0xffff7f7f, 0xffff7f7b,
	0xffff7b7b, 0xffff7b77, 0xffff7777, 0xffff7773, 0xffff7373, 0xffff736f,
	0xffff6f6f, 0xffff6f6b, 0xffff6b6b, 0xffff6b67, 0xffff6767, 0xffff6763,
	0xffff6363, 0xffff635f, 0xffff5f5f, 0xffff5f5b, 0xffff5b5b, 0xffff5b57,
	0xffff5757, 0xffff5753, 0xffff5353, 0xffff534f, 0xffff4f4f, 0xffff4f4b,
	0xffff4b4b, 0xffff4b47, 0xffff4747, 0xffff4743, 0xffff4343, 0xffff433f,
	0xffff3f3f, 0xffff3f3b, 0xffff3b3b, 0xffff3b37, 0xffff3737, 0xffff3733,
	0xffff3333, 0xffff332f, 0xffff2f2f, 0xffff2f2b, 0xffff2b2b, 0xffff2b27,
	0xffff2727, 0xffff2723, 0xffff2323, 0xffff231f, 0xffff1f1f, 0xffff1f1b,
	0xffff1b1b, 0xffff1b17, 0xffff1717, 0xffff1713, 0xffff1313, 0xffff130f,
	0xffff0f0f, 0xffff0f0b, 0xffff0b0b, 0xffff0b07, 0xffff0707, 0xffff0703,
	0xffff0303, 0xffff02ff, 0xfffffefe, 0xfffffefa, 0xfffffafa, 0xfffffaf6,
	0xfffff6f6, 0xfffff6f2, 0xfffff2f2, 0xfffff2ee, 0xffffeeee, 0xffffeeea,
	0xffffeaea, 0xffffeae6, 0xffffe6e6, 0xffffe6e2, 0xffffe2e2, 0xffffe2de,
	0xffffdede, 0xffffdeda, 0xffffdada, 0xffffdad6, 0xffffd6d6, 0xffffd6d2,
	0xffffd2d2, 0xffffd2ce, 0xffffcece, 0xffffceca, 0xffffcaca, 0xffffcac6,
	0xffffc6c6, 0xffffc6c2, 0xffffc2c2, 0xffffc2be, 0xffffbebe, 0xffffbeba,
	0xffffbaba, 0xffffbab6, 0xffffb6b6, 0xffffb6b2, 0xffffb2b2, 0xffffb2ae,
	0xffffaeae, 0xffffaeaa, 0xffffaaaa, 0xffffaaa6, 0xffffa6a6, 0xffffa6a2,
	0xffffa2a2, 0xffffa29e, 0xffff9e9e, 0xffff9e9a, 0xffff9a9a, 0xffff9a96,
	0xffff9696, 0xffff9692, 0xffff9292, 0xffff928e, 0xffff8e8e, 0xffff8e8a,
	0xffff8a8a, 0xffff8a86, 0xffff8686, 0xffff8682, 0xffff8282, 0xffff827e,
	0xffff7e7e, 0xffff7e7a, 0xffff7a7a, 0xffff7a76, 0xffff7676, 0xffff7672,
	0xffff7272, 0xffff726e, 0xffff6e6e, 0xffff6e6a, 0xffff6a6a, 0xffff6a66,
	0xffff6666, 0xffff6662, 0xffff6262, 0xffff625e, 0xffff5e5e, 0xffff5e5a,
	0xffff5a5a, 0xffff5a56, 0xffff5656, 0xffff5652, 0xffff5252, 0xffff524e,
	0xffff4e4e, 0xffff4e4a, 0xffff4a4a, 0xffff4a46, 0xffff4646, 0xffff4642,
	0xffff4242, 0xffff423e, 0xffff3e3e, 0xffff3e3a, 0xffff3a3a, 0xffff3a36,
	0xffff3636, 0xffff3632, 0xffff3232, 0xffff322e, 0xffff2e2e, 0xffff2e2a,
	0xffff2a2a, 0xffff2a26, 0xffff2626, 0xffff2622, 0xffff2222, 0xffff221e,
	0xffff1e1e, 0xffff1e1a, 0xffff1a1a, 0xffff1a16, 0xffff1616, 0xffff1612,
	0xffff1212, 0xffff120e, 0xffff0e0e, 0xffff0e0a, 0xffff0a0a, 0xffff0a06,
	0xffff0606, 0xffff0602, 0xffff0202, 0xffff01fe, 0xfffffdfd, 0xfffffdf9,
	0xfffff9f9, 0xfffff9f5, 0xfffff5f5, 0xfffff5f1, 0xfffff1f1, 0xfffff1ed,
	0xffffeded, 0xffffede9, 0xffffe9e9, 0xffffe9e5, 0xffffe5e5, 0xffffe5e1,
	0xffffe1e1, 0xffffe1dd, 0xffffdddd, 0xffffddd9, 0xffffd9d9, 0xffffd9d5,
	0xffffd5d5, 0xffffd5d1, 0xffffd1d1, 0xffffd1cd, 0xffffcdcd, 0xffffcdc9,
	0xffffc9c9, 0xffffc9c5, 0xffffc5c5, 0xffffc5c1, 0xffffc1c1, 0xffffc1bd,
	0xffffbdbd, 0xffffbdb9, 0xffffb9b9, 0xffffb9b5, 0xffffb5b5, 0xffffb5b1,
	0xffffb1b1, 0xffffb1ad, 0xffffadad, 0xffffada9, 0xffffa9a9, 0xffffa9a5,
	0xffffa5a5, 0xffffa5a1, 0xffffa1a1, 0xffffa19d, 0xffff9d9d, 0xffff9d99,
	0xffff9999, 0xffff9995, 0xffff9595, 0xffff9591, 0xffff9191, 0xffff918d,
	0xffff8d8d, 0xffff8d89, 0xffff8989, 0xffff8985, 0xffff8585, 0xffff8581,
	0xffff8181, 0xffff817d, 0xffff7d7d, 0xffff7d79, 0xffff7979, 0xffff7975,
	0xffff7575, 0xffff7571, 0xffff7171, 0xffff716d, 0xffff6d6d, 0xffff6d69,
	0xffff6969, 0xffff6965, 0xffff6565, 0xffff6561, 0xffff6161, 0xffff615d,
	0xffff5d5d, 0xffff5d59, 0xffff5959, 0xffff5955, 0xffff5555, 0xffff5551,
	0xffff5151, 0xffff514d, 0xffff4d4d, 0xffff4d49, 0xffff4949, 0xffff4945,
	0xffff4545, 0xffff4541, 0xffff4141, 0xffff413d, 0xffff3d3d, 0xffff3d39,
	0xffff3939, 0xffff3935, 0xffff3535, 0xffff3531, 0xffff3131, 0xffff312d,
	0xffff2d2d, 0xffff2d29, 0xffff2929, 0xffff2925, 0xffff2525, 0xffff2521,
	0xffff2121, 0xffff211d, 0xffff1d1d, 0xffff1d19, 0xffff1919, 0xffff1915,
	0xffff1515, 0xffff1511, 0xffff1111, 0xffff110d, 0xffff0d0d, 0xffff0d09,
	0xffff0909, 0xffff0905, 0xffff0505, 0xffff0501, 0xffff0101, 0xffff00fd,
	0xfffffcfc, 0xfffffcf8, 0xfffff8f8, 0xfffff8f4, 0xfffff4f4, 0xfffff4f0,
	0xfffff0f0, 0xfffff0ec, 0xffffecec, 0xffffece8, 0xffffe8e8, 0xffffe8e4,
	0xffffe4e4, 0xffffe4e0, 0xffffe0e0, 0xffffe0dc, 0xffffdcdc, 0xffffdcd8,
	0xffffd8d8, 0xffffd8d4, 0xffffd4d4, 0xffffd4d0, 0xffffd0d0, 0xffffd0cc,
	0xffffcccc, 0xffffccc8, 0xffffc8c8, 0xffffc8c4, 0xffffc4c4, 0xffffc4c0,
	0xffffc0c0, 0xffffc0bc, 0xffffbcbc, 0xffffbcb8, 0xffffb8b8, 0xffffb8b4,
	0xffffb4b4, 0xffffb4b0, 0xffffb0b0, 0xffffb0ac, 0xffffacac, 0xffffaca8,
	0xffffa8a8, 0xffffa8a4, 0xffffa4a4, 0xffffa4a0, 0xffffa0a0, 0xffffa09c,
	0xffff9c9c, 0xffff9c98, 0xffff9898, 0xffff9894, 0xffff9494, 0xffff9490,
	0xffff9090, 0xffff908c, 0xffff8c8c, 0xffff8c88, 0xffff8888, 0xffff8884,
	0xffff8484, 0xffff8480, 0xffff8080, 0xffff807c, 0xffff7c7c, 0xffff7c78,
	0xffff7878, 0xffff7874, 0xffff7474, 0xffff7470, 0xffff7070, 0xffff706c,
	0xffff6c6c, 0xffff6c68, 0xffff6868, 0xffff6864, 0xffff6464, 0xffff6460,
	0xffff6060, 0xffff605c, 0xffff5c5c, 0xffff5c58, 0xffff5858, 0xffff5854,
	0xffff5454, 0xffff5450, 0xffff5050, 0xffff504c, 0xffff4c4c, 0xffff4c48,
	0xffff4848, 0xffff4844, 0xffff4444, 0xffff4440, 0xffff4040, 0xffff403c,
	0xffff3c3c, 0xffff3c38, 0xffff3838, 0xffff3834, 0xffff3434, 0xffff3430,
	0xffff3030, 0xffff302c, 0xffff2c2c, 0xffff2c28, 0xffff2828, 0xffff2824,
	0xffff2424, 0xffff2420, 0xffff2020, 0xffff201c, 0xffff1c1c, 0xffff1c18,
	0xffff1818, 0xffff1814, 0xffff1414, 0xffff1410, 0xffff1010, 0xffff100c,
	0xffff0c0c, 0xffff0c08, 0xffff0808, 0xffff0804, 0xffff0404, 0xffff0400,
	0xffff0000, 0xfffffffb,
};

static const u16 expected_csum_ipv6_magic[] = {
	0x18d4, 0x3085, 0x2e4b, 0xd9f4, 0xbdc8, 0x78f,	0x1034, 0x8422, 0x6fc0,
	0xd2f6, 0xbeb5, 0x9d3,	0x7e2a, 0x312e, 0x778e, 0xc1bb, 0x7cf2, 0x9d1e,
	0xca21, 0xf3ff, 0x7569, 0xb02e, 0xca86, 0x7e76, 0x4539, 0x45e3, 0xf28d,
	0xdf81, 0x8fd5, 0x3b5d, 0x8324, 0xf471, 0x83be, 0x1daf, 0x8c46, 0xe682,
	0xd1fb, 0x6b2e, 0xe687, 0x2a33, 0x4833, 0x2d67, 0x660f, 0x2e79, 0xd65e,
	0x6b62, 0x6672, 0x5dbd, 0x8680, 0xbaa5, 0x2229, 0x2125, 0x2d01, 0x1cc0,
	0x6d36, 0x33c0, 0xee36, 0xd832, 0x9820, 0x8a31, 0x53c5, 0x2e2,	0xdb0e,
	0x49ed, 0x17a7, 0x77a0, 0xd72e, 0x3d72, 0x7dc8, 0x5b17, 0xf55d, 0xa4d9,
	0x1446, 0x5d56, 0x6b2e, 0x69a5, 0xadb6, 0xff2a, 0x92e,	0xe044, 0x3402,
	0xbb60, 0xec7f, 0xe7e6, 0x1986, 0x32f4, 0x8f8,	0x5e00, 0x47c6, 0x3059,
	0x3969, 0xe957, 0x4388, 0x2854, 0x3334, 0xea71, 0xa6de, 0x33f9, 0x83fc,
	0x37b4, 0x5531, 0x3404, 0x1010, 0xed30, 0x610a, 0xc95,	0x9aed, 0x6ff,
	0x5136, 0x2741, 0x660e, 0x8b80, 0xf71,	0xa263, 0x88af, 0x7a73, 0x3c37,
	0x1908, 0x6db5, 0x2e92, 0x1cd2, 0x70c8, 0xee16, 0xe80,	0xcd55, 0x6e6,
	0x6434, 0x127,	0x655d, 0x2ea0, 0xb4f4, 0xdc20, 0x5671, 0xe462, 0xe52b,
	0xdb44, 0x3589, 0xc48f, 0xe60b, 0xd2d2, 0x66ad, 0x498,	0x436,	0xb917,
	0xf0ca, 0x1a6e, 0x1cb7, 0xbf61, 0x2870, 0xc7e8, 0x5b30, 0xe4a5, 0x168,
	0xadfc, 0xd035, 0xe690, 0xe283, 0xfb27, 0xe4ad, 0xb1a5, 0xf2d5, 0xc4b6,
	0x8a30, 0xd7d5, 0x7df9, 0x91d5, 0x63ed, 0x2d21, 0x312b, 0xab19, 0xa632,
	0x8d2e, 0xef06, 0x57b9, 0xc373, 0xbd1f, 0xa41f, 0x8444, 0x9975, 0x90cb,
	0xc49c, 0xe965, 0x4eff, 0x5a,	0xef6d, 0xe81a, 0xe260, 0x853a, 0xff7a,
	0x99aa, 0xb06b, 0xee19, 0xcc2c, 0xf34c, 0x7c49, 0xdac3, 0xa71e, 0xc988,
	0x3845, 0x1014
};

static const u16 expected_fast_csum[] = {
	0xda83, 0x45da, 0x4f46, 0x4e4f, 0x34e,	0xe902, 0xa5e9, 0x87a5, 0x7187,
	0x5671, 0xf556, 0x6df5, 0x816d, 0x8f81, 0xbb8f, 0xfbba, 0x5afb, 0xbe5a,
	0xedbe, 0xabee, 0x6aac, 0xe6b,	0xea0d, 0x67ea, 0x7e68, 0x8a7e, 0x6f8a,
	0x3a70, 0x9f3a, 0xe89e, 0x75e8, 0x7976, 0xfa79, 0x2cfa, 0x3c2c, 0x463c,
	0x7146, 0x7a71, 0x547a, 0xfd53, 0x99fc, 0xb699, 0x92b6, 0xdb91, 0xe8da,
	0x5fe9, 0x1e60, 0xae1d, 0x39ae, 0xf439, 0xa1f4, 0xdda1, 0xede,	0x790f,
	0x579,	0x1206, 0x9012, 0x2490, 0xd224, 0x5cd2, 0xa65d, 0xca7,	0x220d,
	0xf922, 0xbf9,	0x920b, 0x1b92, 0x361c, 0x2e36, 0x4d2e, 0x24d,	0x2,
	0xcfff, 0x90cf, 0xa591, 0x93a5, 0x7993, 0x9579, 0xc894, 0x50c8, 0x5f50,
	0xd55e, 0xcad5, 0xf3c9, 0x8f4,	0x4409, 0x5043, 0x5b50, 0x55b,	0x2205,
	0x1e22, 0x801e, 0x3780, 0xe137, 0x7ee0, 0xf67d, 0x3cf6, 0xa53c, 0x2ea5,
	0x472e, 0x5147, 0xcf51, 0x1bcf, 0x951c, 0x1e95, 0xc71e, 0xe4c7, 0xc3e4,
	0x3dc3, 0xee3d, 0xa4ed, 0xf9a4, 0xcbf8, 0x75cb, 0xb375, 0x50b4, 0x3551,
	0xf835, 0x19f8, 0x8c1a, 0x538c, 0xad52, 0xa3ac, 0xb0a3, 0x5cb0, 0x6c5c,
	0x5b6c, 0xc05a, 0x92c0, 0x4792, 0xbe47, 0x53be, 0x1554, 0x5715, 0x4b57,
	0xe54a, 0x20e5, 0x21,	0xd500, 0xa1d4, 0xa8a1, 0x57a9, 0xca57, 0x5ca,
	0x1c06, 0x4f1c, 0xe24e, 0xd9e2, 0xf0d9, 0x4af1, 0x474b, 0x8146, 0xe81,
	0xfd0e, 0x84fd, 0x7c85, 0xba7c, 0x17ba, 0x4a17, 0x964a, 0xf595, 0xff5,
	0x5310, 0x3253, 0x6432, 0x4263, 0x2242, 0xe121, 0x32e1, 0xf632, 0xc5f5,
	0x21c6, 0x7d22, 0x8e7c, 0x418e, 0x5641, 0x3156, 0x7c31, 0x737c, 0x373,
	0x2503, 0xc22a, 0x3c2,	0x4a04, 0x8549, 0x5285, 0xa352, 0xe8a3, 0x6fe8,
	0x1a6f, 0x211a, 0xe021, 0x38e0, 0x7638, 0xf575, 0x9df5, 0x169e, 0xf116,
	0x23f1, 0xcd23, 0xece,	0x660f, 0x4866, 0x6a48, 0x716a, 0xee71, 0xa2ee,
	0xb8a2, 0x61b9, 0xa361, 0xf7a2, 0x26f7, 0x1127, 0x6611, 0xe065, 0x36e0,
	0x1837, 0x3018, 0x1c30, 0x721b, 0x3e71, 0xe43d, 0x99e4, 0x9e9a, 0xb79d,
	0xa9b7, 0xcaa,	0xeb0c, 0x4eb,	0x1305, 0x8813, 0xb687, 0xa9b6, 0xfba9,
	0xd7fb, 0xccd8, 0x2ecd, 0x652f, 0xae65, 0x3fae, 0x3a40, 0x563a, 0x7556,
	0x2776, 0x1228, 0xef12, 0xf9ee, 0xcef9, 0x56cf, 0xa956, 0x24a9, 0xba24,
	0x5fba, 0x665f, 0xf465, 0x8ff4, 0x6d8f, 0x346d, 0x5f34, 0x385f, 0xd137,
	0xb8d0, 0xacb8, 0x55ac, 0x7455, 0xe874, 0x89e8, 0xd189, 0xa0d1, 0xb2a0,
	0xb8b2, 0x36b8, 0x5636, 0xd355, 0x8d3,	0x1908, 0x2118, 0xc21,	0x990c,
	0x8b99, 0x158c, 0x7815, 0x9e78, 0x6f9e, 0x4470, 0x1d44, 0x341d, 0x2634,
	0x3f26, 0x793e, 0xc79,	0xcc0b, 0x26cc, 0xd126, 0x1fd1, 0xb41f, 0xb6b4,
	0x22b7, 0xa122, 0xa1,	0x7f01, 0x837e, 0x3b83, 0xaf3b, 0x6fae, 0x916f,
	0xb490, 0xffb3, 0xceff, 0x50cf, 0x7550, 0x7275, 0x1272, 0x2613, 0xaa26,
	0xd5aa, 0x7d5,	0x9607, 0x96,	0xb100, 0xf8b0, 0x4bf8, 0xdd4c, 0xeddd,
	0x98ed, 0x2599, 0x9325, 0xeb92, 0x8feb, 0xcc8f, 0x2acd, 0x392b, 0x3b39,
	0xcb3b, 0x6acb, 0xd46a, 0xb8d4, 0x6ab8, 0x106a, 0x2f10, 0x892f, 0x789,
	0xc806, 0x45c8, 0x7445, 0x3c74, 0x3a3c, 0xcf39, 0xd7ce, 0x58d8, 0x6e58,
	0x336e, 0x1034, 0xee10, 0xe9ed, 0xc2e9, 0x3fc2, 0xd53e, 0xd2d4, 0xead2,
	0x8fea, 0x2190, 0x1162, 0xbe11, 0x8cbe, 0x6d8c, 0xfb6c, 0x6dfb, 0xd36e,
	0x3ad3, 0xf3a,	0x870e, 0xc287, 0x53c3, 0xc54,	0x5b0c, 0x7d5a, 0x797d,
	0xec79, 0x5dec, 0x4d5e, 0x184e, 0xd618, 0x60d6, 0xb360, 0x98b3, 0xf298,
	0xb1f2, 0x69b1, 0xf969, 0xef9,	0xab0e, 0x21ab, 0xe321, 0x24e3, 0x8224,
	0x5481, 0x5954, 0x7a59, 0xff7a, 0x7dff, 0x1a7d, 0xa51a, 0x46a5, 0x6b47,
	0xe6b,	0x830e, 0xa083, 0xff9f, 0xd0ff, 0xffd0, 0xe6ff, 0x7de7, 0xc67d,
	0xd0c6, 0x61d1, 0x3a62, 0xc3b,	0x150c, 0x1715, 0x4517, 0x5345, 0x3954,
	0xdd39, 0xdadd, 0x32db, 0x6a33, 0xd169, 0x86d1, 0xb687, 0x3fb6, 0x883f,
	0xa487, 0x39a4, 0x2139, 0xbe20, 0xffbe, 0xedfe, 0x8ded, 0x368e, 0xc335,
	0x51c3, 0x9851, 0xf297, 0xd6f2, 0xb9d6, 0x95ba, 0x2096, 0xea1f, 0x76e9,
	0x4e76, 0xe04d, 0xd0df, 0x80d0, 0xa280, 0xfca2, 0x75fc, 0xef75, 0x32ef,
	0x6833, 0xdf68, 0xc4df, 0x76c4, 0xb77,	0xb10a, 0xbfb1, 0x58bf, 0x5258,
	0x4d52, 0x6c4d, 0x7e6c, 0xb67e, 0xccb5, 0x8ccc, 0xbe8c, 0xc8bd, 0x9ac8,
	0xa99b, 0x52a9, 0x2f53, 0xc30,	0x3e0c, 0xb83d, 0x83b7, 0x5383, 0x7e53,
	0x4f7e, 0xe24e, 0xb3e1, 0x8db3, 0x618e, 0xc861, 0xfcc8, 0x34fc, 0x9b35,
	0xaa9b, 0xb1aa, 0x5eb1, 0x395e, 0x8639, 0xd486, 0x8bd4, 0x558b, 0x2156,
	0xf721, 0x4ef6, 0x14f,	0x7301, 0xdd72, 0x49de, 0x894a, 0x9889, 0x8898,
	0x7788, 0x7b77, 0x637b, 0xb963, 0xabb9, 0x7cab, 0xc87b, 0x21c8, 0xcb21,
	0xdfca, 0xbfdf, 0xf2bf, 0x6af2, 0x626b, 0xb261, 0x3cb2, 0xc63c, 0xc9c6,
	0xc9c9, 0xb4c9, 0xf9b4, 0x91f9, 0x4091, 0x3a40, 0xcc39, 0xd1cb, 0x7ed1,
	0x537f, 0x6753, 0xa167, 0xba49, 0x88ba, 0x7789, 0x3877, 0xf037, 0xd3ef,
	0xb5d4, 0x55b6, 0xa555, 0xeca4, 0xa1ec, 0xb6a2, 0x7b7,	0x9507, 0xfd94,
	0x82fd, 0x5c83, 0x765c, 0x9676, 0x3f97, 0xda3f, 0x6fda, 0x646f, 0x3064,
	0x5e30, 0x655e, 0x6465, 0xcb64, 0xcdca, 0x4ccd, 0x3f4c, 0x243f, 0x6f24,
	0x656f, 0x6065, 0x3560, 0x3b36, 0xac3b, 0x4aac, 0x714a, 0x7e71, 0xda7e,
	0x7fda, 0xda7f, 0x6fda, 0xff6f, 0xc6ff, 0xedc6, 0xd4ed, 0x70d5, 0xeb70,
	0xa3eb, 0x80a3, 0xca80, 0x3fcb, 0x2540, 0xf825, 0x7ef8, 0xf87e, 0x73f8,
	0xb474, 0xb4b4, 0x92b5, 0x9293, 0x93,	0x3500, 0x7134, 0x9071, 0xfa8f,
	0x51fa, 0x1452, 0xba13, 0x7ab9, 0x957a, 0x8a95, 0x6e8a, 0x6d6e, 0x7c6d,
	0x447c, 0x9744, 0x4597, 0x8945, 0xef88, 0x8fee, 0x3190, 0x4831, 0x8447,
	0xa183, 0x1da1, 0xd41d, 0x2dd4, 0x4f2e, 0xc94e, 0xcbc9, 0xc9cb, 0x9ec9,
	0x319e, 0xd531, 0x20d5, 0x4021, 0xb23f, 0x29b2, 0xd828, 0xecd8, 0x5ded,
	0xfc5d, 0x4dfc, 0xd24d, 0x6bd2, 0x5f6b, 0xb35e, 0x7fb3, 0xee7e, 0x56ee,
	0xa657, 0x68a6, 0x8768, 0x7787, 0xb077, 0x4cb1, 0x764c, 0xb175, 0x7b1,
	0x3d07, 0x603d, 0x3560, 0x3e35, 0xb03d, 0xd6b0, 0xc8d6, 0xd8c8, 0x8bd8,
	0x3e8c, 0x303f, 0xd530, 0xf1d4, 0x42f1, 0xca42, 0xddca, 0x41dd, 0x3141,
	0x132,	0xe901, 0x8e9,	0xbe09, 0xe0bd, 0x2ce0, 0x862d, 0x3986, 0x9139,
	0x6d91, 0x6a6d, 0x8d6a, 0x1b8d, 0xac1b, 0xedab, 0x54ed, 0xc054, 0xcebf,
	0xc1ce, 0x5c2,	0x3805, 0x6038, 0x5960, 0xd359, 0xdd3,	0xbe0d, 0xafbd,
	0x6daf, 0x206d, 0x2c20, 0x862c, 0x8e86, 0xec8d, 0xa2ec, 0xa3a2, 0x51a3,
	0x8051, 0xfd7f, 0x91fd, 0xa292, 0xaf14, 0xeeae, 0x59ef, 0x535a, 0x8653,
	0x3986, 0x9539, 0xb895, 0xa0b8, 0x26a0, 0x2227, 0xc022, 0x77c0, 0xad77,
	0x46ad, 0xaa46, 0x60aa, 0x8560, 0x4785, 0xd747, 0x45d7, 0x2346, 0x5f23,
	0x25f,	0x1d02, 0x71d,	0x8206, 0xc82,	0x180c, 0x3018, 0x4b30, 0x4b,
	0x3001, 0x1230, 0x2d12, 0x8c2d, 0x148d, 0x4015, 0x5f3f, 0x3d5f, 0x6b3d,
	0x396b, 0x473a, 0xf746, 0x44f7, 0x8945, 0x3489, 0xcb34, 0x84ca, 0xd984,
	0xf0d9, 0xbcf0, 0x63bd, 0x3264, 0xf332, 0x45f3, 0x7346, 0x5673, 0xb056,
	0xd3b0, 0x4ad4, 0x184b, 0x7d18, 0x6c7d, 0xbb6c, 0xfeba, 0xe0fe, 0x10e1,
	0x5410, 0x2954, 0x9f28, 0x3a9f, 0x5a3a, 0xdb59, 0xbdc,	0xb40b, 0x1ab4,
	0x131b, 0x5d12, 0x6d5c, 0xe16c, 0xb0e0, 0x89b0, 0xba88, 0xbb,	0x3c01,
	0xe13b, 0x6fe1, 0x446f, 0xa344, 0x81a3, 0xfe81, 0xc7fd, 0x38c8, 0xb38,
	0x1a0b, 0x6d19, 0xf36c, 0x47f3, 0x6d48, 0xb76d, 0xd3b7, 0xd8d2, 0x52d9,
	0x4b53, 0xa54a, 0x34a5, 0xc534, 0x9bc4, 0xed9b, 0xbeed, 0x3ebe, 0x233e,
	0x9f22, 0x4a9f, 0x774b, 0x4577, 0xa545, 0x64a5, 0xb65,	0x870b, 0x487,
	0x9204, 0x5f91, 0xd55f, 0x35d5, 0x1a35, 0x71a,	0x7a07, 0x4e7a, 0xfc4e,
	0x1efc, 0x481f, 0x7448, 0xde74, 0xa7dd, 0x1ea7, 0xaa1e, 0xcfaa, 0xfbcf,
	0xedfb, 0x6eee, 0x386f, 0x4538, 0x6e45, 0xd96d, 0x11d9, 0x7912, 0x4b79,
	0x494b, 0x6049, 0xac5f, 0x65ac, 0x1366, 0x5913, 0xe458, 0x7ae4, 0x387a,
	0x3c38, 0xb03c, 0x76b0, 0x9376, 0xe193, 0x42e1, 0x7742, 0x6476, 0x3564,
	0x3c35, 0x6a3c, 0xcc69, 0x94cc, 0x5d95, 0xe5e,	0xee0d, 0x4ced, 0xce4c,
	0x52ce, 0xaa52, 0xdaaa, 0xe4da, 0x1de5, 0x4530, 0x5445, 0x3954, 0xb639,
	0x81b6, 0x7381, 0x1574, 0xc215, 0x10c2, 0x3f10, 0x6b3f, 0xe76b, 0x7be7,
	0xbc7b, 0xf7bb, 0x41f7, 0xcc41, 0x38cc, 0x4239, 0xa942, 0x4a9,	0xc504,
	0x7cc4, 0x437c, 0x6743, 0xea67, 0x8dea, 0xe88d, 0xd8e8, 0xdcd8, 0x17dd,
	0x5718, 0x958,	0xa609, 0x41a5, 0x5842, 0x159,	0x9f01, 0x269f, 0x5a26,
	0x405a, 0xc340, 0xb4c3, 0xd4b4, 0xf4d3, 0xf1f4, 0x39f2, 0xe439, 0x67e4,
	0x4168, 0xa441, 0xdda3, 0xdedd, 0x9df,	0xab0a, 0xa5ab, 0x9a6,	0xba09,
	0x9ab9, 0xad9a, 0x5ae,	0xe205, 0xece2, 0xecec, 0x14ed, 0xd614, 0x6bd5,
	0x916c, 0x3391, 0x6f33, 0x206f, 0x8020, 0x780,	0x7207, 0x2472, 0x8a23,
	0xb689, 0x3ab6, 0xf739, 0x97f6, 0xb097, 0xa4b0, 0xe6a4, 0x88e6, 0x2789,
	0xb28,	0x350b, 0x1f35, 0x431e, 0x1043, 0xc30f, 0x79c3, 0x379,	0x5703,
	0x3256, 0x4732, 0x7247, 0x9d72, 0x489d, 0xd348, 0xa4d3, 0x7ca4, 0xbf7b,
	0x45c0, 0x7b45, 0x337b, 0x4034, 0x843f, 0xd083, 0x35d0, 0x6335, 0x4d63,
	0xe14c, 0xcce0, 0xfecc, 0x35ff, 0x5636, 0xf856, 0xeef8, 0x2def, 0xfc2d,
	0x4fc,	0x6e04, 0xb66d, 0x78b6, 0xbb78, 0x3dbb, 0x9a3d, 0x839a, 0x9283,
	0x593,	0xd504, 0x23d5, 0x5424, 0xd054, 0x61d0, 0xdb61, 0x17db, 0x1f18,
	0x381f, 0x9e37, 0x679e, 0x1d68, 0x381d, 0x8038, 0x917f, 0x491,	0xbb04,
	0x23bb, 0x4124, 0xd41,	0xa30c, 0x8ba3, 0x8b8b, 0xc68b, 0xd2c6, 0xebd2,
	0x93eb, 0xbd93, 0x99bd, 0x1a99, 0xea19, 0x58ea, 0xcf58, 0x73cf, 0x1073,
	0x9e10, 0x139e, 0xea13, 0xcde9, 0x3ecd, 0x883f, 0xf89,	0x180f, 0x2a18,
	0x212a, 0xce20, 0x73ce, 0xf373, 0x60f3, 0xad60, 0x4093, 0x8e40, 0xb98e,
	0xbfb9, 0xf1bf, 0x8bf1, 0x5e8c, 0xe95e, 0x14e9, 0x4e14, 0x1c4e, 0x7f1c,
	0xe77e, 0x6fe7, 0xf26f, 0x13f2, 0x8b13, 0xda8a, 0x5fda, 0xea5f, 0x4eea,
	0xa84f, 0x88a8, 0x1f88, 0x2820, 0x9728, 0x5a97, 0x3f5b, 0xb23f, 0x70b2,
	0x2c70, 0x232d, 0xf623, 0x4f6,	0x905,	0x7509, 0xd675, 0x28d7, 0x9428,
	0x3794, 0xf036, 0x2bf0, 0xba2c, 0xedb9, 0xd7ed, 0x59d8, 0xed59, 0x4ed,
	0xe304, 0x18e3, 0x5c19, 0x3d5c, 0x753d, 0x6d75, 0x956d, 0x7f95, 0xc47f,
	0x83c4, 0xa84,	0x2e0a, 0x5f2e, 0xb95f, 0x77b9, 0x6d78, 0xf46d, 0x1bf4,
	0xed1b, 0xd6ed, 0xe0d6, 0x5e1,	0x3905, 0x5638, 0xa355, 0x99a2, 0xbe99,
	0xb4bd, 0x85b4, 0x2e86, 0x542e, 0x6654, 0xd765, 0x73d7, 0x3a74, 0x383a,
	0x2638, 0x7826, 0x7677, 0x9a76, 0x7e99, 0x2e7e, 0xea2d, 0xa6ea, 0x8a7,
	0x109,	0x3300, 0xad32, 0x5fad, 0x465f, 0x2f46, 0xc62f, 0xd4c5, 0xad5,
	0xcb0a, 0x4cb,	0xb004, 0x7baf, 0xe47b, 0x92e4, 0x8e92, 0x638e, 0x1763,
	0xc17,	0xf20b, 0x1ff2, 0x8920, 0x5889, 0xcb58, 0xf8cb, 0xcaf8, 0x84cb,
	0x9f84, 0x8a9f, 0x918a, 0x4991, 0x8249, 0xff81, 0x46ff, 0x5046, 0x5f50,
	0x725f, 0xf772, 0x8ef7, 0xe08f, 0xc1e0, 0x1fc2, 0x9e1f, 0x8b9d, 0x108b,
	0x411,	0x2b04, 0xb02a, 0x1fb0, 0x1020, 0x7a0f, 0x587a, 0x8958, 0xb188,
	0xb1b1, 0x49b2, 0xb949, 0x7ab9, 0x917a, 0xfc91, 0xe6fc, 0x47e7, 0xbc47,
	0x8fbb, 0xea8e, 0x34ea, 0x2635, 0x1726, 0x9616, 0xc196, 0xa6c1, 0xf3a6,
	0x11f3, 0x4811, 0x3e48, 0xeb3e, 0xf7ea, 0x1bf8, 0xdb1c, 0x8adb, 0xe18a,
	0x42e1, 0x9d42, 0x5d9c, 0x6e5d, 0x286e, 0x4928, 0x9a49, 0xb09c, 0xa6b0,
	0x2a7,	0xe702, 0xf5e6, 0x9af5, 0xf9b,	0x810f, 0x8080, 0x180,	0x1702,
	0x5117, 0xa650, 0x11a6, 0x1011, 0x550f, 0xd554, 0xbdd5, 0x6bbe, 0xc66b,
	0xfc7,	0x5510, 0x5555, 0x7655, 0x177,	0x2b02, 0x6f2a, 0xb70,	0x9f0b,
	0xcf9e, 0xf3cf, 0x3ff4, 0xcb40, 0x8ecb, 0x768e, 0x5277, 0x8652, 0x9186,
	0x9991, 0x5099, 0xd350, 0x93d3, 0x6d94, 0xe6d,	0x530e, 0x3153, 0xa531,
	0x64a5, 0x7964, 0x7c79, 0x467c, 0x1746, 0x3017, 0x3730, 0x538,	0x5,
	0x1e00, 0x5b1e, 0x955a, 0xae95, 0x3eaf, 0xff3e, 0xf8ff, 0xb2f9, 0xa1b3,
	0xb2a1, 0x5b2,	0xad05, 0x7cac, 0x2d7c, 0xd32c, 0x80d2, 0x7280, 0x8d72,
	0x1b8e, 0x831b, 0xac82, 0xfdac, 0xa7fd, 0x15a8, 0xd614, 0xe0d5, 0x7be0,
	0xb37b, 0x61b3, 0x9661, 0x9d95, 0xc79d, 0x83c7, 0xd883, 0xead7, 0xceb,
	0xf60c, 0xa9f5, 0x19a9, 0xa019, 0x8f9f, 0xd48f, 0x3ad5, 0x853a, 0x985,
	0x5309, 0x6f52, 0x1370, 0x6e13, 0xa96d, 0x98a9, 0x5198, 0x9f51, 0xb69f,
	0xa1b6, 0x2ea1, 0x672e, 0x2067, 0x6520, 0xaf65, 0x6eaf, 0x7e6f, 0xee7e,
	0x17ef, 0xa917, 0xcea8, 0x9ace, 0xff99, 0x5dff, 0xdf5d, 0x38df, 0xa39,
	0x1c0b, 0xe01b, 0x46e0, 0xcb46, 0x90cb, 0xba90, 0x4bb,	0x9104, 0x9d90,
	0xc89c, 0xf6c8, 0x6cf6, 0x886c, 0x1789, 0xbd17, 0x70bc, 0x7e71, 0x17e,
	0x1f01, 0xa01f, 0xbaa0, 0x14bb, 0xfc14, 0x7afb, 0xa07a, 0x3da0, 0xbf3d,
	0x48bf, 0x8c48, 0x968b, 0x9d96, 0xfd9d, 0x96fd, 0x9796, 0x6b97, 0xd16b,
	0xf4d1, 0x3bf4, 0x253c, 0x9125, 0x6691, 0xc166, 0x34c1, 0x5735, 0x1a57,
	0xdc19, 0x77db, 0x8577, 0x4a85, 0x824a, 0x9182, 0x7f91, 0xfd7f, 0xb4c3,
	0xb5b4, 0xb3b5, 0x7eb3, 0x617e, 0x4e61, 0xa4f,	0x530a, 0x3f52, 0xa33e,
	0x34a3, 0x9234, 0xf091, 0xf4f0, 0x1bf5, 0x311b, 0x9631, 0x6a96, 0x386b,
	0x1d39, 0xe91d, 0xe8e9, 0x69e8, 0x426a, 0xee42, 0x89ee, 0x368a, 0x2837,
	0x7428, 0x5974, 0x6159, 0x1d62, 0x7b1d, 0xf77a, 0x7bf7, 0x6b7c, 0x696c,
	0xf969, 0x4cf9, 0x714c, 0x4e71, 0x6b4e, 0x256c, 0x6e25, 0xe96d, 0x94e9,
	0x8f94, 0x3e8f, 0x343e, 0x4634, 0xb646, 0x97b5, 0x8997, 0xe8a,	0x900e,
	0x8090, 0xfd80, 0xa0fd, 0x16a1, 0xf416, 0xebf4, 0x95ec, 0x1196, 0x8911,
	0x3d89, 0xda3c, 0x9fd9, 0xd79f, 0x4bd7, 0x214c, 0x3021, 0x4f30, 0x994e,
	0x5c99, 0x6f5d, 0x326f, 0xab31, 0x6aab, 0xe969, 0x90e9, 0x1190, 0xff10,
	0xa2fe, 0xe0a2, 0x66e1, 0x4067, 0x9e3f, 0x2d9e, 0x712d, 0x8170, 0xd180,
	0xffd1, 0x25ff, 0x3826, 0x2538, 0x5f24, 0xc45e, 0x1cc4, 0xdf1c, 0x93df,
	0xc793, 0x80c7, 0x2380, 0xd223, 0x7ed2, 0xfc7e, 0x22fd, 0x7422, 0x1474,
	0xb714, 0x7db6, 0x857d, 0xa85,	0xa60a, 0x88a6, 0x4289, 0x7842, 0xc278,
	0xf7c2, 0xcdf7, 0x84cd, 0xae84, 0x8cae, 0xb98c, 0x1aba, 0x4d1a, 0x884c,
	0x4688, 0xcc46, 0xd8cb, 0x2bd9, 0xbe2b, 0xa2be, 0x72a2, 0xf772, 0xd2f6,
	0x75d2, 0xc075, 0xa3c0, 0x63a3, 0xae63, 0x8fae, 0x2a90, 0x5f2a, 0xef5f,
	0x5cef, 0xa05c, 0x89a0, 0x5e89, 0x6b5e, 0x736b, 0x773,	0x9d07, 0xe99c,
	0x27ea, 0x2028, 0xc20,	0x980b, 0x4797, 0x2848, 0x9828, 0xc197, 0x48c2,
	0x2449, 0x7024, 0x570,	0x3e05, 0xd3e,	0xf60c, 0xbbf5, 0x69bb, 0x3f6a,
	0x740,	0xf006, 0xe0ef, 0xbbe0, 0xadbb, 0x56ad, 0xcf56, 0xbfce, 0xa9bf,
	0x205b, 0x6920, 0xae69, 0x50ae, 0x2050, 0xf01f, 0x27f0, 0x9427, 0x8993,
	0x8689, 0x4087, 0x6e40, 0xb16e, 0xa1b1, 0xe8a1, 0x87e8, 0x6f88, 0xfe6f,
	0x4cfe, 0xe94d, 0xd5e9, 0x47d6, 0x3148, 0x5f31, 0xc35f, 0x13c4, 0xa413,
	0x5a5,	0x2405, 0xc223, 0x66c2, 0x3667, 0x5e37, 0x5f5e, 0x2f5f, 0x8c2f,
	0xe48c, 0xd0e4, 0x4d1,	0xd104, 0xe4d0, 0xcee4, 0xfcf,	0x480f, 0xa447,
	0x5ea4, 0xff5e, 0xbefe, 0x8dbe, 0x1d8e, 0x411d, 0x1841, 0x6918, 0x5469,
	0x1155, 0xc611, 0xaac6, 0x37ab, 0x2f37, 0xca2e, 0x87ca, 0xbd87, 0xabbd,
	0xb3ab, 0xcb4,	0xce0c, 0xfccd, 0xa5fd, 0x72a5, 0xf072, 0x83f0, 0xfe83,
	0x97fd, 0xc997, 0xb0c9, 0xadb0, 0xe6ac, 0x88e6, 0x1088, 0xbe10, 0x16be,
	0xa916, 0xa3a8, 0x46a3, 0x5447, 0xe953, 0x84e8, 0x2085, 0xa11f, 0xfa1,
	0xdd0f, 0xbedc, 0x5abe, 0x805a, 0xc97f, 0x6dc9, 0x826d, 0x4a82, 0x934a,
	0x5293, 0xd852, 0xd3d8, 0xadd3, 0xf4ad, 0xf3f4, 0xfcf3, 0xfefc, 0xcafe,
	0xb7ca, 0x3cb8, 0xa13c, 0x18a1, 0x1418, 0xea13, 0x91ea, 0xf891, 0x53f8,
	0xa254, 0xe9a2, 0x87ea, 0x4188, 0x1c41, 0xdc1b, 0xf5db, 0xcaf5, 0x45ca,
	0x6d45, 0x396d, 0xde39, 0x90dd, 0x1e91, 0x1e,	0x7b00, 0x6a7b, 0xa46a,
	0xc9a3, 0x9bc9, 0x389b, 0x1139, 0x5211, 0x1f52, 0xeb1f, 0xabeb, 0x48ab,
	0x9348, 0xb392, 0x17b3, 0x1618, 0x5b16, 0x175b, 0xdc17, 0xdedb, 0x1cdf,
	0xeb1c, 0xd1ea, 0x4ad2, 0xd4b,	0xc20c, 0x24c2, 0x7b25, 0x137b, 0x8b13,
	0x618b, 0xa061, 0xff9f, 0xfffe, 0x72ff, 0xf572, 0xe2f5, 0xcfe2, 0xd2cf,
	0x75d3, 0x6a76, 0xc469, 0x1ec4, 0xfc1d, 0x59fb, 0x455a, 0x7a45, 0xa479,
	0xb7a4
};

static u8 tmp_buf[TEST_BUFLEN];

#define full_csum(buff, len, sum) csum_fold(csum_partial(buff, len, sum))

#define CHECK_EQ(lhs, rhs) KUNIT_ASSERT_EQ(test, (__force u64)lhs, (__force u64)rhs)

static __sum16 to_sum16(u16 x)
{
	return (__force __sum16)le16_to_cpu((__force __le16)x);
}

/* This function swaps the bytes inside each half of a __wsum */
static __wsum to_wsum(u32 x)
{
	u16 hi = le16_to_cpu((__force __le16)(x >> 16));
	u16 lo = le16_to_cpu((__force __le16)x);

	return (__force __wsum)((hi << 16) | lo);
}

static void assert_setup_correct(struct kunit *test)
{
	CHECK_EQ(sizeof(random_buf) / sizeof(random_buf[0]), MAX_LEN);
	CHECK_EQ(sizeof(expected_results) / sizeof(expected_results[0]),
		 MAX_LEN);
	CHECK_EQ(sizeof(init_sums_no_overflow) /
			 sizeof(init_sums_no_overflow[0]),
		 MAX_LEN);
}

/*
 * Test with randomized input (pre determined random with known results).
 */
static void test_csum_fixed_random_inputs(struct kunit *test)
{
	int len, align;
	__wsum sum;
	__sum16 result, expec;

	assert_setup_correct(test);
	for (align = 0; align < TEST_BUFLEN; ++align) {
		memcpy(&tmp_buf[align], random_buf,
		       min(MAX_LEN, TEST_BUFLEN - align));
		for (len = 0; len < MAX_LEN && (align + len) < TEST_BUFLEN;
		     ++len) {
			/*
			 * Test the precomputed random input.
			 */
			sum = to_wsum(random_init_sum);
			result = full_csum(&tmp_buf[align], len, sum);
			expec = to_sum16(expected_results[len]);
			CHECK_EQ(result, expec);
		}
	}
}

/*
 * All ones input test. If there are any missing carry operations, it fails.
 */
static void test_csum_all_carry_inputs(struct kunit *test)
{
	int len, align;
	__wsum sum;
	__sum16 result, expec;

	assert_setup_correct(test);
	memset(tmp_buf, 0xff, TEST_BUFLEN);
	for (align = 0; align < TEST_BUFLEN; ++align) {
		for (len = 0; len < MAX_LEN && (align + len) < TEST_BUFLEN;
		     ++len) {
			/*
			 * All carries from input and initial sum.
			 */
			sum = to_wsum(0xffffffff);
			result = full_csum(&tmp_buf[align], len, sum);
			expec = to_sum16((len & 1) ? 0xff00 : 0);
			CHECK_EQ(result, expec);

			/*
			 * All carries from input.
			 */
			sum = 0;
			result = full_csum(&tmp_buf[align], len, sum);
			if (len & 1)
				expec = to_sum16(0xff00);
			else if (len)
				expec = 0;
			else
				expec = to_sum16(0xffff);
			CHECK_EQ(result, expec);
		}
	}
}

/*
 * Test with input that alone doesn't cause any carries. By selecting the
 * maximum initial sum, this allows us to test that there are no carries
 * where there shouldn't be.
 */
static void test_csum_no_carry_inputs(struct kunit *test)
{
	int len, align;
	__wsum sum;
	__sum16 result, expec;

	assert_setup_correct(test);
	memset(tmp_buf, 0x4, TEST_BUFLEN);
	for (align = 0; align < TEST_BUFLEN; ++align) {
		for (len = 0; len < MAX_LEN && (align + len) < TEST_BUFLEN;
		     ++len) {
			/*
			 * Expect no carries.
			 */
			sum = to_wsum(init_sums_no_overflow[len]);
			result = full_csum(&tmp_buf[align], len, sum);
			expec = 0;
			CHECK_EQ(result, expec);

			/*
			 * Expect one carry.
			 */
			sum = to_wsum(init_sums_no_overflow[len] + 1);
			result = full_csum(&tmp_buf[align], len, sum);
			expec = to_sum16(len ? 0xfffe : 0xffff);
			CHECK_EQ(result, expec);
		}
	}
}

static void test_ip_fast_csum(struct kunit *test)
{
	__sum16 csum_result;
	u16 expected;

	for (int len = IPv4_MIN_WORDS; len < IPv4_MAX_WORDS; len++) {
		for (int index = 0; index < NUM_IP_FAST_CSUM_TESTS; index++) {
			csum_result = ip_fast_csum(random_buf + index, len);
			expected =
				expected_fast_csum[(len - IPv4_MIN_WORDS) *
						   NUM_IP_FAST_CSUM_TESTS +
						   index];
			CHECK_EQ(to_sum16(expected), csum_result);
		}
	}
}

static void test_csum_ipv6_magic(struct kunit *test)
{
#if defined(CONFIG_NET)
	const struct in6_addr *saddr;
	const struct in6_addr *daddr;
	unsigned int len;
	unsigned char proto;
	__wsum csum;

	const int daddr_offset = sizeof(struct in6_addr);
	const int len_offset = sizeof(struct in6_addr) + sizeof(struct in6_addr);
	const int proto_offset = sizeof(struct in6_addr) + sizeof(struct in6_addr) +
			     sizeof(int);
	const int csum_offset = sizeof(struct in6_addr) + sizeof(struct in6_addr) +
			    sizeof(int) + sizeof(char);

	for (int i = 0; i < NUM_IPv6_TESTS; i++) {
		saddr = (const struct in6_addr *)(random_buf + i);
		daddr = (const struct in6_addr *)(random_buf + i +
						  daddr_offset);
		len = le32_to_cpu(*(__le32 *)(random_buf + i + len_offset));
		proto = *(random_buf + i + proto_offset);
		csum = *(__wsum *)(random_buf + i + csum_offset);
		CHECK_EQ(to_sum16(expected_csum_ipv6_magic[i]),
			 csum_ipv6_magic(saddr, daddr, len, proto, csum));
	}
#endif /* !CONFIG_NET */
}

static struct kunit_case __refdata checksum_test_cases[] = {
	KUNIT_CASE(test_csum_fixed_random_inputs),
	KUNIT_CASE(test_csum_all_carry_inputs),
	KUNIT_CASE(test_csum_no_carry_inputs),
	KUNIT_CASE(test_ip_fast_csum),
	KUNIT_CASE(test_csum_ipv6_magic),
	{}
};

static struct kunit_suite checksum_test_suite = {
	.name = "checksum",
	.test_cases = checksum_test_cases,
};

kunit_test_suites(&checksum_test_suite);

MODULE_AUTHOR("Noah Goldstein <goldstein.w.n@gmail.com>");
MODULE_LICENSE("GPL");
