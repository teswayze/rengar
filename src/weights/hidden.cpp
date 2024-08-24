# include "hidden.hpp"

const Eigen::Vector<float, 32> w_l0_tempo_va = { 0.0032,  0.0246,  0.0930, -0.0513, -0.0028, -0.0097, -0.0111, -0.0614, -0.0020,  0.0080, -0.0082,  0.0197,  0.0075,  0.0027,  0.0044,  0.0081,  0.0282,  0.0122,  0.0220,  0.0252, -0.0634, -0.0199, -0.0044,  0.0072, -0.0190, -0.0123, -0.0005, -0.0163, -0.0527,  0.0452, -0.0069, -0.0194};

const Eigen::Vector4f w_l1_fs_bias = {-0.0955, -0.2072, -0.0847,  0.0256};

const Eigen::Matrix<float, 4, 32, Eigen::RowMajor> w_l1_fs {
	{ 0.0061,  0.1736,  0.0196,  0.1063, -0.0407, -0.1645,  0.0882,  0.3231,  0.0865,  0.0569, -0.0568,  0.5523,  0.0348, -0.0139,  0.2082,  0.1803, -0.4992,  0.0136,  0.0195, -0.0228,  0.2441,  0.0987,  0.0893,  0.0311, -0.1233,  0.0262, -0.0238,  0.1621,  0.8096,  0.0838, -0.2271, -0.5899},
	{ 0.0688, -0.0138,  0.0931,  0.1298, -0.0055,  0.1162, -0.0661,  0.1470, -0.0679,  0.0653,  0.0033, -0.8116,  0.0225, -0.0902,  0.1629, -0.4489, -0.1509,  0.0983, -0.0703, -0.1359,  0.0477, -0.0959,  0.1179, -0.0625,  0.0727, -0.1078,  0.0754, -0.2406,  0.3218, -0.1700, -0.2035, -0.1319},
	{-0.2184,  0.3869,  0.2294,  0.3226,  0.0951,  0.2250,  0.1919, -0.1614, -0.0986,  0.1135,  0.1296,  0.2231,  0.0878,  0.0283, -0.7162,  0.1030, -0.0955, -0.0035,  0.0128, -0.1678,  0.0504,  0.4047, -0.0527, -0.3422,  0.3989, -0.0804,  0.0163,  0.0299,  0.0976, -0.0867, -0.0353, -1.1168},
	{ 0.0076, -0.1917,  0.0398, -0.0397, -0.1485, -0.1692, -0.2801,  0.1736,  0.0647,  0.0801,  0.0133, -0.4481, -0.1181,  0.0060, -0.3170, -0.3221,  0.0674, -0.0966,  0.2179,  0.0662, -0.3418, -0.0052, -0.0743,  0.1767,  0.0716,  0.0800,  0.0091, -0.2533, -0.0725, -0.0320,  0.1147,  0.0888}
};

const Eigen::Matrix<float, 4, 32, Eigen::RowMajor> w_l1_absva {
	{-0.0448,  0.0816, -0.0369, -0.0168, -0.0731, -0.0664,  0.0549,  0.0939, -0.2898, -0.1261, -0.0605, -0.0627, -0.0508,  0.1270,  0.1336, -0.1577, -0.0459,  0.1967,  0.0067,  0.0460,  0.0650, -0.0769, -0.1503, -0.1052,  0.1930,  0.0254, -0.0582, -0.1712, -0.0334,  0.0107,  0.0168, -0.1091},
	{ 0.2445, -0.0320,  0.0021,  0.2175,  0.0856,  0.1655, -0.0930, -0.2031,  0.5143,  0.4665,  0.4181,  0.0230,  0.2417, -0.2767, -0.7286,  0.5090,  0.4975, -0.1562,  0.0502,  0.0392, -0.0592,  0.2984,  0.5554,  0.4076, -0.4427,  0.2195,  0.2961,  0.3921,  0.3549,  0.0725,  0.1032,  0.5480},
	{ 0.0619,  0.1487, -0.0119,  0.0297, -0.1198,  0.1400,  0.0967,  0.1019, -0.0226, -0.0558,  0.0451, -0.1112, -0.0232,  0.1734,  0.0472,  0.0482, -0.1583,  0.0874,  0.1372, -0.0548,  0.0022, -0.0359, -0.2884,  0.0794,  0.0222, -0.1862,  0.1185,  0.0054, -0.0074, -0.0162,  0.1196, -0.0174},
	{ 0.1524,  0.0524,  0.0299,  0.1210,  0.0643,  0.1549,  0.0011, -0.0722,  0.3652,  0.1085,  0.1589,  0.2400,  0.1177, -0.1248, -0.2290,  0.1575,  0.3122, -0.0725, -0.0306,  0.0063,  0.0496,  0.1540,  0.2787,  0.1766, -0.1377,  0.1828,  0.1079,  0.2286,  0.2308, -0.0232,  0.1030,  0.2733}
};

const Eigen::Matrix<float, 4, 32, Eigen::RowMajor> w_l1_absha {
	{ 0.0533,  0.0881,  0.0111,  0.0837,  0.0170, -0.1697,  0.0424,  0.0293,  0.0529, -0.0578,  0.2247,  0.2281,  0.0978,  0.0827, -0.0303,  0.0872,  0.0740, -0.0283,  0.1704,  0.0240,  0.0235,  0.1357, -0.0398,  0.1250, -0.0114, -0.1291, -0.0960, -0.0033, -0.0762,  0.0624,  0.2039,  0.1124},
	{-0.0783, -0.1059, -0.1417, -0.1825, -0.0154,  0.1790, -0.0879,  0.0146, -0.1842,  0.0446, -0.3967, -0.4193, -0.0924, -0.2828,  0.0071, -0.2832, -0.2157,  0.0251, -0.5220,  0.0158,  0.0908, -0.3668,  0.0293, -0.4013,  0.0153,  0.1446,  0.0873,  0.0485, -0.0688, -0.2841, -0.2911, -0.2650},
	{ 0.0563,  0.1015, -0.0241,  0.0784,  0.0935, -0.1126,  0.0718, -0.0808,  0.0400, -0.0678,  0.0965,  0.2183, -0.0360, -0.1020,  0.0011,  0.1376,  0.1516,  0.1173,  0.0592,  0.0631, -0.0275,  0.0411,  0.0419,  0.0984, -0.0152,  0.1248, -0.0888, -0.1183,  0.1467, -0.0004,  0.1783,  0.1119},
	{-0.0502, -0.0758, -0.0780, -0.2511, -0.0095,  0.2556, -0.0251,  0.0932, -0.0843,  0.1323, -0.1249, -0.1605, -0.0835,  0.0895,  0.0372,  0.0058, -0.1376,  0.1711, -0.0863,  0.0551,  0.0346, -0.1620,  0.0875, -0.1186, -0.0044,  0.1867,  0.0823,  0.1319,  0.0508, -0.0984, -0.2338, -0.2449}
};

const Eigen::Matrix<float, 4, 32, Eigen::RowMajor> w_l1_absra {
	{ 0.0184, -0.0045, -0.0261,  0.0417, -0.0189,  0.0756, -0.0785, -0.1331,  0.1291, -0.0300, -0.0351, -0.0308,  0.0733,  0.3295, -0.0126,  0.0030,  0.0945,  0.0208,  0.0989, -0.0051, -0.0094,  0.0630,  0.0431,  0.0081,  0.0810, -0.0361,  0.0251, -0.0040,  0.0726,  0.0274,  0.0549,  0.0132},
	{-0.0547,  0.0006,  0.0335,  0.0308,  0.0960,  0.0048,  0.0319,  0.1682, -0.0793,  0.0616,  0.0569,  0.1428, -0.1048, -0.1253, -0.0876, -0.1204, -0.1411,  0.1644, -0.0182,  0.0565,  0.0060, -0.1471, -0.0699, -0.0598, -0.2124,  0.0345,  0.0791,  0.3264, -0.0935, -0.1559, -0.1024, -0.0314},
	{ 0.1228, -0.0334, -0.0010,  0.0989,  0.0350, -0.0090, -0.0822,  0.0804,  0.0978,  0.0361,  0.1139, -0.0809,  0.0047, -0.1047,  0.1161,  0.0595,  0.0699, -0.0786, -0.1494, -0.0122,  0.0289,  0.0175,  0.0291,  0.1180,  0.0799,  0.1005, -0.0358,  0.0283,  0.0791,  0.0223,  0.0417,  0.1390},
	{-0.0805, -0.0617,  0.0638,  0.0682,  0.1370,  0.0493,  0.0874,  0.2037, -0.0209,  0.0215,  0.0399,  0.1289, -0.1790, -0.0358, -0.0664, -0.0505, -0.1817,  0.0844, -0.0479, -0.0437,  0.0018, -0.1486, -0.0533, -0.0313, -0.1349,  0.0815,  0.0230,  0.0510,  0.0501, -0.0912, -0.1072,  0.0136}
};

const Eigen::Matrix<float, 4, 32, Eigen::RowMajor> w_l1_va {
	{-0.0723, -0.1254, -0.2048,  0.0200,  0.0254, -0.1220,  0.0527,  0.0461,  0.1747,  0.2172,  0.0327, -0.4710, -0.0326, -0.1303, -0.2352, -0.0638,  0.2693, -0.0440,  0.0474,  0.0088, -0.0258, -0.2025,  0.4862, -0.0938, -0.0853,  0.4281, -0.1570,  0.0408, -0.4643, -0.1419,  0.0907,  0.0522},
	{ 0.0483, -0.0401,  0.0608,  0.2218, -0.0275,  0.0641,  0.0054,  0.0088,  0.4998,  0.2697,  0.0668, -0.4504, -0.1631,  0.0543, -0.2273, -0.2010,  0.4162,  0.0812, -0.0954, -0.0287,  0.2207, -0.2638,  0.4121, -0.1481, -0.0723,  0.4851, -0.1119,  0.3164, -0.3546, -0.0931,  0.1094,  0.3884},
	{-0.1583, -0.0476,  0.0726, -0.0262, -0.0495, -0.3356,  0.0896, -0.0625, -0.2209, -0.1058, -0.1069, -0.1596,  0.2010, -0.1475, -0.4505,  0.1733, -0.2239, -0.1675,  0.1490, -0.1181, -0.0187,  0.1558, -0.2583, -0.0628,  0.3110, -0.2481,  0.2249, -0.3101,  0.1169,  0.0626, -0.0574, -0.3070},
	{ 0.0007, -0.0641,  0.0062,  0.1647, -0.0112,  0.0711,  0.0625, -0.0697,  0.3351,  0.3260, -0.0059, -0.6559,  0.0227, -0.0282, -0.2591, -0.0503,  0.3200, -0.0331, -0.0221, -0.0061, -0.1089, -0.3801,  0.6198, -0.0371, -0.1175,  0.4427, -0.2024, -0.0112, -0.4289, -0.1074,  0.2618,  0.2547}
};

const Eigen::Matrix<float, 4, 32, Eigen::RowMajor> w_l1_fsxva {
	{-0.0142,  0.1129,  0.1238, -0.0256,  0.1547, -0.1636, -0.0970,  0.1375, -0.0913,  0.0708, -0.1141,  0.5979, -0.0548,  0.0881,  0.6665, -0.0795,  0.1872, -0.0654, -0.0040, -0.0343, -0.0500,  0.1110, -0.4529, -0.0439,  0.2862,  0.1770, -0.1071,  0.1648,  0.6460, -0.1152,  0.0227,  0.1897},
	{-0.0251,  0.1020,  0.2377, -0.0456,  0.1822, -0.1949, -0.0779,  0.1500,  0.1629,  0.1302,  0.0264,  0.3023,  0.0789,  0.0253, -0.0026,  0.0506,  0.2388, -0.1907, -0.0753, -0.0598, -0.0866,  0.1798, -0.1490, -0.0574,  0.1145,  0.0700,  0.0366, -0.1622,  0.2839, -0.1378, -0.0129,  0.2715},
	{ 0.0032, -0.0973, -0.0543,  0.0392, -0.1232,  0.0721, -0.0057, -0.3269, -0.0684, -0.0206,  0.0689, -0.2935,  0.0029, -0.0046, -0.5826,  0.1454, -0.1731,  0.1563, -0.0354,  0.0393,  0.0951, -0.1298, -0.1703,  0.0934, -0.1320, -0.1869, -0.1067, -0.0018, -0.2362,  0.0720, -0.0345, -0.2814},
	{-0.1223,  0.1422,  0.0439, -0.2025,  0.1871, -0.2116, -0.0709,  0.2052,  0.1476,  0.1408,  0.0643,  0.7694, -0.1617,  0.0478,  0.5582, -0.1378,  0.2835, -0.0906, -0.2311,  0.0231, -0.0063,  0.0763, -0.3041, -0.1243,  0.1069,  0.1897,  0.0837, -0.0334,  0.5215, -0.2380,  0.0088,  0.1683}
};

const Eigen::Matrix<float, 4, 32, Eigen::RowMajor> w_l1_haxra {
	{-0.0858,  0.0776, -0.0077,  0.2230,  0.0572,  0.0402,  0.1146,  0.0519,  0.0830, -0.0489, -0.0110, -0.1181, -0.0026, -0.0535,  0.0916,  0.0648, -0.0218, -0.0324, -0.3668,  0.2693, -0.5852,  0.2711, -0.0694,  0.2105, -0.0035, -0.0752,  0.1756, -0.4498, -0.0071,  0.0274, -0.1410,  0.0297},
	{-0.0348,  0.0865,  0.1801, -0.1615,  0.0285,  0.1018,  0.1350, -0.0960, -0.3528, -0.1780, -0.1404,  0.2245,  0.0405,  0.0197, -0.2026,  0.0741,  0.0885, -0.2108, -0.0081, -0.4193, -0.0933,  0.0379,  0.0552,  0.0857,  0.0754,  0.3092,  0.1483,  0.1878,  0.1609, -0.0754,  0.0237, -0.0244},
	{ 0.0664, -0.1202,  0.1063,  0.0695,  0.0278, -0.0643, -0.0643, -0.0173,  0.1197,  0.0299,  0.0551, -0.0814, -0.0820, -0.7653,  0.0085, -0.0549,  0.0238, -0.0682,  0.4950,  0.0276,  0.3426, -0.1661, -0.0427, -0.1852,  0.0518, -0.0361, -0.1459,  0.0153, -0.0186,  0.2781,  0.0221, -0.0193},
	{ 0.0662,  0.2307,  0.1510, -0.1567,  0.1715, -0.0581,  0.2163,  0.0574, -0.1868, -0.0775, -0.2495,  0.0481, -0.0523,  0.4053, -0.1204,  0.2992, -0.1034, -0.1068, -0.2994,  0.0773, -0.0718,  0.3421,  0.1652,  0.3401,  0.1067,  0.0921,  0.1542,  0.0378,  0.0639, -0.0848, -0.0203, -0.1753}
};

const Eigen::Vector4f w_l2_va = {-0.5722, -0.1652,  0.6049, -0.6724};

const Eigen::Vector4f w_l2_fsxva = { 0.6447, -0.5941, -0.5705, -0.7280};
