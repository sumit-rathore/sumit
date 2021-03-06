# NOTE: this file and rom/src/authentication.c must be kept in sync.
keys = (
    (
        0x54, 0xf5, 0x20, 0x16,
        0x77, 0xf7, 0xe5, 0xa7,
        0xf9, 0x57, 0x9c, 0xf2,
        0x43, 0x7c, 0xc5, 0x54,
        0xd2, 0xdd, 0x18, 0xb3,
        0xc5, 0x10, 0xd4, 0x4d,
        0x99, 0xc1, 0x09, 0xa7,
        0xe1, 0x35, 0x16, 0x28
    ),
    (
        0xd7, 0xf6, 0x42, 0x29,
        0x6e, 0x0f, 0xc9, 0x3a,
        0x77, 0x54, 0x03, 0xf2,
        0x82, 0xa8, 0x60, 0x06,
        0x65, 0xeb, 0xda, 0x06,
        0xcd, 0xa8, 0x32, 0xca,
        0x27, 0xb8, 0x43, 0x05,
        0xd0, 0x18, 0x1f, 0xdb
    ),
    (
        0x2d, 0xed, 0x8c, 0x1e,
        0xc3, 0x64, 0x68, 0x6a,
        0xbb, 0xdb, 0xcd, 0x88,
        0xea, 0x07, 0x95, 0x3b,
        0x63, 0x14, 0xef, 0xb3,
        0xd4, 0xf3, 0xd3, 0x2f,
        0x91, 0xb8, 0xd2, 0xac,
        0x2d, 0xea, 0xd6, 0xb3
    ),
    (
        0x30, 0x21, 0xca, 0xe2,
        0xff, 0x7f, 0x3c, 0x57,
        0x46, 0xc6, 0xcb, 0x7b,
        0xa0, 0xa8, 0xce, 0x67,
        0x10, 0x73, 0xfe, 0x64,
        0xd9, 0x03, 0x01, 0xe5,
        0xa7, 0x23, 0x7a, 0xef,
        0x8c, 0xe8, 0x02, 0xdb
    ),
    (
        0x9c, 0x82, 0x1c, 0x5e,
        0xc5, 0xe0, 0x84, 0xc7,
        0xf2, 0x78, 0xba, 0xd2,
        0xbe, 0x13, 0xc6, 0x12,
        0x03, 0x79, 0x0d, 0x21,
        0x7c, 0x52, 0x38, 0x81,
        0x11, 0xf6, 0x8e, 0xee,
        0x3e, 0x6e, 0x01, 0x0c
    ),
    (
        0x26, 0x13, 0x84, 0xe5,
        0x0a, 0x4a, 0x43, 0xce,
        0xa3, 0x96, 0xea, 0xd1,
        0xde, 0xbe, 0x99, 0x42,
        0xff, 0xe5, 0x30, 0xbd,
        0xd9, 0x65, 0xb3, 0xb9,
        0x93, 0x15, 0xbf, 0x1a,
        0x84, 0xcb, 0x08, 0x11
    ),
    (
        0x2d, 0x79, 0xfb, 0x33,
        0xc4, 0x95, 0x8f, 0x6c,
        0xba, 0x8b, 0x05, 0x54,
        0x41, 0x4b, 0x99, 0xb1,
        0x07, 0x3e, 0xfd, 0x90,
        0xc6, 0x8e, 0xdb, 0xd8,
        0x4f, 0xae, 0xe8, 0xf1,
        0x36, 0xb9, 0x11, 0xd9
    ),
    (
        0x41, 0x44, 0x5a, 0x2c,
        0xbe, 0x48, 0x3d, 0xff,
        0xcf, 0xf6, 0xe9, 0x02,
        0x1d, 0x13, 0x2a, 0x1d,
        0x5b, 0xae, 0xd0, 0xe7,
        0x36, 0xe8, 0x67, 0x9e,
        0x52, 0xfc, 0xea, 0xb3,
        0xfc, 0xad, 0x58, 0xfe
    ),
    (
        0x54, 0x24, 0x84, 0x30,
        0x47, 0xca, 0xa6, 0xb1,
        0x7a, 0x42, 0x66, 0x06,
        0x75, 0x59, 0x31, 0x6f,
        0x3a, 0xaa, 0x23, 0xf7,
        0x03, 0xbf, 0x9a, 0xdc,
        0x33, 0x58, 0x4f, 0x80,
        0x47, 0x64, 0xe0, 0x81
    ),
    (
        0xd3, 0x0a, 0xc0, 0xb0,
        0x8c, 0x50, 0x1c, 0xce,
        0x7b, 0x8c, 0xc9, 0xdc,
        0x36, 0xca, 0x51, 0x87,
        0x4b, 0xcf, 0x14, 0xb8,
        0x43, 0x4b, 0x37, 0x0c,
        0x60, 0x0c, 0xb8, 0xd0,
        0x27, 0x3f, 0xe3, 0xaa
    ),
    (
        0xe2, 0xc5, 0x7f, 0xfa,
        0x56, 0x5d, 0x6a, 0x58,
        0xd6, 0x4a, 0x0c, 0xce,
        0x3d, 0xca, 0x5e, 0x19,
        0xa6, 0xa7, 0xef, 0xc1,
        0x74, 0xc2, 0x6e, 0x43,
        0x81, 0x30, 0x6a, 0x6a,
        0x8a, 0xf0, 0xe0, 0xb0
    ),
    (
        0xb7, 0xa3, 0xb7, 0x38,
        0xed, 0xde, 0xe6, 0xfe,
        0xc9, 0xbd, 0x96, 0x02,
        0x21, 0x0b, 0xa4, 0xbb,
        0xe8, 0x82, 0x12, 0xce,
        0xf5, 0x69, 0x45, 0x0f,
        0x69, 0x68, 0xdc, 0x54,
        0xc0, 0xf2, 0x7a, 0xa0
    ),
    (
        0x88, 0xb5, 0x09, 0xdb,
        0xb6, 0x13, 0xef, 0x75,
        0x57, 0x0c, 0xe5, 0x27,
        0x82, 0xd9, 0xbf, 0x36,
        0xa2, 0x18, 0x8e, 0x15,
        0xe9, 0xf7, 0x9b, 0xb6,
        0xed, 0x04, 0x55, 0xc8,
        0x51, 0x13, 0x80, 0xc0
    ),
    (
        0x61, 0x0c, 0xc8, 0x50,
        0x29, 0x73, 0xf9, 0xa7,
        0xb0, 0x01, 0xa6, 0xf0,
        0x28, 0x9e, 0x6f, 0xc9,
        0xc8, 0x5a, 0x74, 0x5e,
        0x26, 0x97, 0x2d, 0x9f,
        0x99, 0x7d, 0xac, 0x67,
        0x3d, 0x41, 0x5e, 0xaf
    ),
    (
        0xdc, 0xc0, 0xc4, 0x1c,
        0x6e, 0xc4, 0x3f, 0x5d,
        0x54, 0xfe, 0x76, 0xcb,
        0x9a, 0xd8, 0x1d, 0xd7,
        0xb3, 0x41, 0x00, 0x8d,
        0x96, 0xc9, 0x62, 0x88,
        0x4c, 0x84, 0xf2, 0x4b,
        0x29, 0xc4, 0xa9, 0x4f
    ),
    (
        0xc6, 0x86, 0x0f, 0x7b,
        0x1e, 0x24, 0xb9, 0xb7,
        0x6d, 0x8e, 0xea, 0x34,
        0x72, 0xcf, 0xa9, 0x80,
        0x5d, 0x56, 0x6d, 0x88,
        0x6f, 0xa4, 0x5c, 0x8e,
        0x88, 0x25, 0xe3, 0x44,
        0xaa, 0xc3, 0x97, 0x72
    )
)
