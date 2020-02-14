const struct IdtClkRegMap IDT6914_lexRexUsb3IdtClkCfg[] =
{
};
const uint8_t IDT6914_lexRexUsb3IdtClkCfgSize = sizeof(IDT6914_lexRexUsb3IdtClkCfg) / sizeof(IDT6914_lexRexUsb3IdtClkCfg[0]);

const struct IdtClkRegMap IDT6914_rexUsb3DpIdtClkCfg[] =
{
    {.regOffset = 0x21, .regValue = 0x81},
    {.regOffset = 0x61, .regValue = 0x1},
    {.regOffset = 0x69, .regValue = 0xf4}
};
const uint8_t IDT6914_rexUsb3DpIdtClkCfgSize = sizeof(IDT6914_rexUsb3DpIdtClkCfg) / sizeof(IDT6914_rexUsb3DpIdtClkCfg[0]);

const struct IdtClkRegMap IDT6914_rexUsb3DpSscEnableIdtClkCfg[] =
{
    {.regOffset = 0x21, .regValue = 0x81},
    {.regOffset = 0x25, .regValue = 0x46},
    {.regOffset = 0x27, .regValue = 0x1},
    {.regOffset = 0x28, .regValue = 0x6a},
    {.regOffset = 0x29, .regValue = 0x42},
    {.regOffset = 0x2a, .regValue = 0xfc},
    {.regOffset = 0x61, .regValue = 0x1},
    {.regOffset = 0x69, .regValue = 0xf4}
};
const uint8_t IDT6914_rexUsb3DpSscEnableIdtClkCfgSize = sizeof(IDT6914_rexUsb3DpSscEnableIdtClkCfg) / sizeof(IDT6914_rexUsb3DpSscEnableIdtClkCfg[0]);

const struct IdtClkRegMap IDT6914_lexUsb2IdtClkCfg[] =
{
    {.regOffset = 0x22, .regValue = 0x0},
    {.regOffset = 0x23, .regValue = 0x0},
    {.regOffset = 0x24, .regValue = 0x0},
    {.regOffset = 0x25, .regValue = 0x0},
    {.regOffset = 0x2e, .regValue = 0xe0},
    {.regOffset = 0x41, .regValue = 0x0},
    {.regOffset = 0x42, .regValue = 0x0},
    {.regOffset = 0x45, .regValue = 0x0},
    {.regOffset = 0x47, .regValue = 0x0},
    {.regOffset = 0x48, .regValue = 0x0},
    {.regOffset = 0x49, .regValue = 0x0},
    {.regOffset = 0x4a, .regValue = 0x4},
    {.regOffset = 0x4d, .regValue = 0x0},
    {.regOffset = 0x4e, .regValue = 0x0},
    {.regOffset = 0x60, .regValue = 0xbb},
    {.regOffset = 0x64, .regValue = 0xbb},
    {.regOffset = 0x65, .regValue = 0x0},
    {.regOffset = 0x69, .regValue = 0xa4}
};
const uint8_t IDT6914_lexUsb2IdtClkCfgSize = sizeof(IDT6914_lexUsb2IdtClkCfg) / sizeof(IDT6914_lexUsb2IdtClkCfg[0]);

const struct IdtClkRegMap IDT6914_rexUsb2DpIdtClkCfg[] =
{
    {.regOffset = 0x21, .regValue = 0x81},
    {.regOffset = 0x41, .regValue = 0x0},
    {.regOffset = 0x42, .regValue = 0x0},
    {.regOffset = 0x45, .regValue = 0x0},
    {.regOffset = 0x47, .regValue = 0x0},
    {.regOffset = 0x48, .regValue = 0x0},
    {.regOffset = 0x49, .regValue = 0x0},
    {.regOffset = 0x4a, .regValue = 0x4},
    {.regOffset = 0x4d, .regValue = 0x0},
    {.regOffset = 0x4e, .regValue = 0x0},
    {.regOffset = 0x61, .regValue = 0x1},
    {.regOffset = 0x64, .regValue = 0xbb},
    {.regOffset = 0x65, .regValue = 0x0},
    {.regOffset = 0x69, .regValue = 0xe4}
};
const uint8_t IDT6914_rexUsb2DpIdtClkCfgSize = sizeof(IDT6914_rexUsb2DpIdtClkCfg) / sizeof(IDT6914_rexUsb2DpIdtClkCfg[0]);

const struct IdtClkRegMap IDT6914_rexUsb2DpSscEnableIdtClkCfg[] =
{
    {.regOffset = 0x21, .regValue = 0x81},
    {.regOffset = 0x25, .regValue = 0x46},
    {.regOffset = 0x27, .regValue = 0x1},
    {.regOffset = 0x28, .regValue = 0x6a},
    {.regOffset = 0x29, .regValue = 0x42},
    {.regOffset = 0x2a, .regValue = 0xfc},
    {.regOffset = 0x41, .regValue = 0x0},
    {.regOffset = 0x42, .regValue = 0x0},
    {.regOffset = 0x45, .regValue = 0x0},
    {.regOffset = 0x47, .regValue = 0x0},
    {.regOffset = 0x48, .regValue = 0x0},
    {.regOffset = 0x49, .regValue = 0x0},
    {.regOffset = 0x4a, .regValue = 0x4},
    {.regOffset = 0x4d, .regValue = 0x0},
    {.regOffset = 0x4e, .regValue = 0x0},
    {.regOffset = 0x61, .regValue = 0x1},
    {.regOffset = 0x64, .regValue = 0xbb},
    {.regOffset = 0x65, .regValue = 0x0},
    {.regOffset = 0x69, .regValue = 0xe4}
};
const uint8_t IDT6914_rexUsb2DpSscEnableIdtClkCfgSize = sizeof(IDT6914_rexUsb2DpSscEnableIdtClkCfg) / sizeof(IDT6914_rexUsb2DpSscEnableIdtClkCfg[0]);

