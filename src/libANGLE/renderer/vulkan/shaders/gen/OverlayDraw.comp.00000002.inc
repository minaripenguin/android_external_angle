// GENERATED FILE - DO NOT EDIT.
// Generated by gen_vk_internal_shaders.py.
//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// shaders/gen/OverlayDraw.comp.00000002.inc:
//   Pre-generated shader for the ANGLE Vulkan back-end.

#pragma once
constexpr uint8_t kOverlayDraw_comp_00000002[] = {
    0x1f,0x8b,0x08,0x00,0x00,0x00,0x00,0x00,0x02,0xff,0x75,0x58,0x7d,0x90,0x97,0x55,
    0x15,0x7e,0xcf,0xfd,0xed,0x2e,0xbb,0x69,0x92,0x29,0xa0,0xb6,0xa2,0xac,0x28,0xc8,
    0xe6,0xba,0x24,0x1f,0x36,0x1b,0x1f,0x0b,0xca,0x22,0x9f,0x29,0x4c,0xc5,0x42,0x11,
    0x1f,0x23,0xc9,0x00,0x6b,0x40,0x7c,0x53,0x28,0x88,0x81,0xa5,0xbb,0x0b,0xae,0x05,
    0x51,0xfd,0xe3,0x44,0xd0,0x34,0xfe,0x51,0x33,0xa6,0x34,0x13,0xb2,0xa9,0x30,0x8b,
    0x08,0x0d,0x51,0x42,0x64,0xa0,0x12,0x42,0x4d,0x2a,0xb8,0xa8,0x53,0x74,0x9f,0xf7,
    0x3c,0x87,0xf7,0xec,0x3b,0xcb,0x6f,0xe6,0xce,0xfb,0xde,0xe7,0x9c,0x7b,0xce,0x73,
    0xcf,0x3d,0xf7,0xde,0xf3,0x7b,0x0b,0xe1,0x96,0x2e,0x49,0x22,0xc9,0xa7,0x92,0xd2,
    0xa4,0xa4,0x90,0xa4,0xbf,0xab,0x93,0x10,0x91,0x24,0xb9,0x22,0x29,0x49,0x9f,0x75,
    0xe3,0x26,0x8d,0xab,0x5a,0xb4,0x78,0x76,0xd5,0x80,0x81,0xd5,0x90,0x5f,0x95,0xa8,
    0x22,0x64,0x5d,0x93,0x2e,0x49,0x71,0x7c,0x16,0xc5,0x36,0x7f,0xc6,0x43,0x0b,0x80,
    0x1f,0x8c,0xed,0x74,0x6c,0x9f,0x89,0xe3,0x8b,0x52,0x7b,0x2a,0x2f,0xe2,0x98,0xba,
    0xf8,0x36,0x3c,0x3e,0x4b,0x52,0x9d,0x24,0x19,0x1d,0x2d,0xd4,0xaa,0xeb,0xe4,0x16,
    0x3e,0x0d,0x13,0x62,0x5e,0x2f,0x10,0xbb,0xd9,0x61,0x05,0x62,0xd5,0xb4,0x3f,0x82,
    0xf6,0xab,0x45,0x75,0x46,0xe6,0xec,0xd7,0xc5,0x11,0x23,0x69,0x0b,0xfa,0xf7,0xc6,
    0x67,0xc5,0x25,0x99,0xf6,0x7b,0x39,0xbe,0xab,0x73,0xf2,0xd5,0x94,0x17,0xb1,0xdf,
    0x94,0x9b,0x4f,0x73,0x27,0xf3,0x69,0xee,0x64,0x3e,0xcd,0x9d,0xcc,0xa7,0xb9,0x93,
    0xf9,0x6c,0xb2,0xf9,0x14,0xa9,0xce,0xe6,0x4e,0xe6,0xb3,0xd9,0xcd,0xa7,0x25,0xc7,
    0xb7,0x85,0x7c,0x4d,0x7e,0x30,0x5d,0xdf,0x24,0x29,0xa7,0xcf,0xc3,0x9d,0xf0,0x3d,
    0xec,0xf8,0x96,0xd2,0xc7,0x61,0x67,0xe3,0x54,0xce,0xc7,0x29,0xfa,0x28,0xb0,0x7f,
    0x9a,0x3e,0x6e,0x60,0xff,0x4d,0xe9,0xa8,0x8f,0x7e,0x2f,0xd7,0x3f,0x27,0xaa,0x7f,
    0x7d,0x6c,0xd7,0x44,0x2f,0x21,0xb5,0x57,0x48,0xed,0xe1,0xbd,0x5b,0xd4,0x29,0x61,
    0x9c,0xf0,0xeb,0x11,0xfb,0x5d,0x18,0x97,0xc0,0x7e,0x29,0xfb,0x58,0x97,0x6b,0x23,
    0x5a,0x16,0x9f,0xdd,0xa3,0x85,0x6e,0x1c,0x07,0x9d,0x8a,0xd4,0x96,0xea,0xa0,0x7f,
    0x7b,0x7c,0x96,0xd1,0x46,0x25,0x7d,0x0c,0xa2,0x0f,0xeb,0xd7,0x50,0xdf,0xfa,0xc3,
    0xb8,0x86,0xe5,0xcc,0xe5,0x52,0x62,0x3d,0xa3,0xb4,0x96,0xfd,0x0a,0x3e,0x87,0x53,
    0x0f,0x39,0x59,0x7b,0x49,0x4f,0xf3,0x6f,0x44,0xca,0xab,0x28,0xb9,0x87,0xfe,0x47,
    0xa6,0xbe,0xb4,0x7f,0xaf,0x9b,0xf7,0x28,0xf2,0x17,0x72,0x18,0x95,0xc6,0x2c,0xe3,
    0x38,0x2a,0x5d,0x33,0x8d,0x3d,0xec,0x8d,0xe5,0xd8,0x52,0xc7,0xb9,0xde,0x8d,0x47,
    0x7f,0x66,0x6e,0xce,0x0d,0x39,0xfd,0xc5,0xb1,0x5d,0x4c,0xd7,0xa3,0x2c,0x59,0xc9,
    0x98,0x09,0x7d,0xe6,0x9f,0xf0,0xb9,0x8a,0xef,0x2b,0x39,0x07,0xf4,0x57,0xbb,0xb5,
    0x5a,0x13,0x9f,0xa3,0xdc,0x5a,0xad,0x65,0xbf,0xe0,0x7c,0x36,0xba,0x78,0x35,0x91,
    0x4f,0x23,0xe3,0xda,0x9c,0x8b,0x6b,0x13,0xf5,0xb0,0x37,0x9a,0x5d,0x5c,0xb1,0x0f,
    0x36,0x91,0xd3,0xd3,0xf4,0xb7,0x99,0x9c,0xd0,0x6f,0x71,0x1c,0x5e,0xe0,0xbc,0x8c,
    0x03,0xde,0x77,0xa5,0x8c,0xd7,0x0c,0x83,0xfc,0x00,0xf3,0xc9,0xe2,0xfa,0x3a,0xe7,
    0x7c,0x80,0xf6,0x5e,0xe7,0x79,0x27,0xa9,0xef,0xa2,0x74,0x7f,0x58,0x4e,0x42,0xff,
    0xcf,0xcc,0xad,0xc3,0xd4,0x47,0xff,0x08,0x31,0xc8,0xff,0xc2,0xf7,0x2e,0xce,0xff,
    0x09,0xc6,0xec,0xf3,0x11,0xc5,0x5c,0x4f,0x12,0xb3,0xb6,0x8b,0x6b,0xf2,0x36,0xfd,
    0xd8,0x1a,0x24,0x9d,0xac,0xc9,0x3b,0x7c,0x7f,0x9b,0xfe,0xd1,0x3f,0x45,0xcc,0xf8,
    0x9f,0xce,0xe5,0x15,0x3a,0xe2,0xfc,0x0f,0x94,0x8e,0xfe,0x4f,0x38,0xdd,0xa1,0xa2,
    0xb1,0x84,0xaf,0x61,0x7c,0xb7,0xb9,0x7f,0x9b,0xfd,0x0a,0xb7,0xbe,0xdb,0x23,0x36,
    0xdd,0xcd,0xf5,0x37,0xb1,0xbf,0xbf,0xad,0x6d,0xa8,0xf9,0xfa,0x6d,0xce,0x17,0xe4,
    0xb0,0xf5,0x47,0xd1,0x38,0x95,0x70,0xee,0xc7,0xa5,0x63,0x3e,0xda,0x2f,0x10,0xc3,
    0x98,0xbf,0x53,0x08,0xdd,0x1a,0xf6,0xdf,0x14,0x8b,0x6d,0x49,0xba,0x86,0xe7,0x44,
    0xf7,0x76,0x0d,0xf7,0x47,0x55,0x5c,0x69,0xac,0x45,0x71,0x01,0x67,0x40,0x71,0x6a,
    0xaf,0x88,0xb6,0x91,0x03,0x17,0x22,0x82,0xfb,0xae,0x3d,0xf6,0xce,0xd3,0xd6,0x27,
    0xf1,0x1d,0xe7,0xc5,0x87,0xa2,0x72,0x3c,0x87,0x30,0x77,0x0e,0x31,0x3f,0x26,0x46,
    0xab,0xb0,0xfb,0x27,0x62,0x87,0xdc,0x5a,0xd5,0x46,0x8b,0xc8,0x83,0xbf,0x32,0x37,
    0xb0,0xaf,0x87,0xf0,0x6c,0x7b,0x83,0xf8,0xaf,0xa3,0x0e,0xce,0xa9,0xa3,0xb4,0x01,
    0x7c,0x4b,0xd4,0x41,0x4c,0x8e,0x11,0x07,0xa7,0xe3,0xb4,0xfb,0x71,0x94,0x01,0xff,
    0x1b,0xe6,0x4f,0xde,0x78,0xff,0x28,0x3e,0xcf,0x93,0xe7,0x71,0xfa,0x41,0x6e,0xfc,
    0x93,0x79,0x61,0xbc,0xdf,0x65,0x5e,0x18,0xef,0x33,0xc4,0xde,0x75,0xbc,0x57,0x71,
    0x4f,0x9f,0xa5,0x7c,0x7a,0xd4,0xc4,0xbe,0xfc,0x17,0xed,0x9d,0xbd,0x74,0x57,0x24,
    0xc9,0xfd,0x91,0x3f,0xd6,0xee,0x3d,0xca,0x13,0x87,0x9d,0x23,0x26,0xc6,0x8f,0x7c,
    0xf1,0x3c,0xcf,0xbc,0x68,0x0d,0xba,0x0f,0xc0,0xb9,0x3d,0x40,0xae,0x32,0x8c,0xff,
    0x43,0x50,0x1b,0x90,0x15,0x8b,0xca,0x76,0x46,0xdb,0x65,0xe9,0xda,0xa8,0x3c,0x5d,
    0x9f,0xc8,0xb7,0x3d,0x51,0xb9,0xc5,0xe8,0x93,0x14,0x4f,0x88,0x87,0xf4,0xfd,0xca,
    0x68,0x15,0xb9,0xfd,0x3f,0x72,0xba,0x8f,0x36,0x56,0x31,0x87,0x2f,0x52,0xd6,0x1a,
    0x7d,0xa4,0xe7,0x99,0xa8,0xe2,0xc5,0x24,0xd3,0x29,0x88,0xe2,0x2f,0x73,0x8e,0xe0,
    0x05,0x1b,0xc0,0x6b,0x23,0x6f,0x9c,0xcf,0xa5,0xa2,0x67,0x51,0x1d,0x6d,0xda,0xba,
    0x23,0x86,0x65,0xa2,0x72,0x8b,0xbf,0x04,0xc5,0xca,0x24,0x8b,0xbf,0xe5,0x44,0x08,
    0x9a,0x13,0xd0,0xd9,0xca,0x9c,0x28,0x04,0xc5,0x91,0x13,0xa5,0x21,0x9b,0x2f,0xf0,
    0xa2,0xd8,0x4a,0x83,0xce,0x17,0xef,0xe6,0xa3,0xd8,0xf9,0x08,0xcc,0xf7,0xe7,0xe8,
    0xa3,0x84,0x3e,0x8a,0x9d,0x8f,0x2e,0x41,0x71,0xac,0x99,0xd9,0xc3,0x13,0xeb,0x02,
    0x79,0x59,0x50,0x1e,0x1f,0x52,0x17,0xbe,0xb6,0x73,0xec,0xa7,0x45,0xe5,0xe0,0xd7,
    0x55,0x32,0x7e,0xc0,0xaf,0x8a,0xad,0x2b,0x73,0x14,0xef,0xb0,0x7f,0x81,0xeb,0x03,
    0xdc,0xf2,0xe6,0xb3,0x92,0xed,0x23,0xc3,0xae,0x75,0x31,0xda,0xc1,0x1c,0xe8,0x26,
    0xaa,0x0b,0x99,0xf9,0xef,0x2e,0x8a,0xc3,0xff,0x75,0xce,0x3f,0xf0,0x1e,0xb1,0x5d,
    0x47,0xff,0x3d,0x9c,0xbf,0x1b,0xe8,0x4f,0x9c,0xbf,0x72,0xfa,0x13,0xe7,0xef,0x46,
    0x51,0xdd,0x72,0x72,0x37,0x5b,0x78,0x5a,0x6c,0x7a,0xd2,0x3f,0xe6,0x73,0x23,0x7d,
    0x1a,0xb7,0x9b,0x44,0xe5,0xe0,0xd6,0xcb,0x71,0x03,0x7e,0x33,0x6a,0x26,0xda,0xc3,
    0xfb,0x1a,0xf2,0xe8,0xcd,0x39,0xd6,0x3b,0x6e,0xb7,0xb9,0xb5,0x34,0x6e,0x7d,0x44,
    0x75,0x6f,0x23,0x37,0xb3,0xd5,0xcb,0x71,0xeb,0x4b,0xff,0xe0,0xdb,0x87,0x3e,0x8d,
    0xdb,0xed,0xa2,0x72,0x70,0xab,0x74,0xdc,0x80,0xf7,0x8b,0xad,0x92,0xf6,0xfa,0xb9,
    0xb8,0xdd,0xe1,0xe2,0x66,0x7c,0xab,0x44,0x71,0xcf,0xb7,0x9a,0x7c,0x0b,0x8e,0x6f,
    0x7f,0x51,0xdd,0x6a,0xf2,0x35,0xfb,0x95,0x8e,0xef,0x17,0xc8,0x09,0x73,0xe8,0x4f,
    0x1e,0xe0,0x37,0xc0,0xf1,0x83,0xce,0x5d,0xb1,0x0d,0xe0,0xf8,0xbb,0x72,0x79,0x05,
    0xfc,0xd1,0xe8,0x13,0xfb,0xa0,0x46,0xb2,0xfd,0x54,0x1b,0x77,0x23,0xee,0xb5,0xe1,
    0xb9,0xbd,0x8a,0x7b,0x6f,0x10,0xf7,0x2b,0xb8,0xd7,0x52,0x07,0x7e,0xfb,0x87,0x8e,
    0xf7,0xc2,0xd5,0xdc,0x1b,0x78,0x5e,0xca,0xdd,0xa0,0x7e,0xf0,0x5b,0x4f,0xec,0x9a,
    0xa0,0x38,0x6c,0x19,0xd6,0x2d,0x28,0x5e,0x93,0x28,0x17,0x9c,0x1b,0xdd,0x43,0x47,
    0x2e,0xa3,0xa9,0x67,0x67,0x47,0x8f,0xa0,0x3a,0x1b,0x68,0xe3,0x7a,0x67,0xc3,0xe2,
    0xfa,0xb9,0xa0,0xf8,0x20,0xde,0x1b,0xd5,0xee,0x8c,0x80,0xac,0x3c,0xb6,0x9b,0xc8,
    0x1b,0x4f,0x1b,0xd7,0x8b,0xe3,0xea,0x39,0xee,0x4e,0x37,0x0e,0xb2,0x8a,0xd8,0x6e,
    0xe5,0xb8,0x5b,0xdd,0xb8,0x3e,0x1c,0x37,0x93,0xe3,0xaa,0xdc,0x38,0xc8,0xfa,0xc6,
    0x56,0xc9,0x71,0x95,0x2e,0x4e,0x77,0x04,0x9d,0x4f,0x81,0x77,0x43,0x7f,0xea,0xf4,
    0x75,0x3a,0xfd,0xa8,0x13,0x72,0x3a,0xf0,0x71,0x31,0x66,0x1d,0xde,0x2b,0x9c,0x7e,
    0x6f,0xea,0x4b,0x4e,0xff,0x4e,0xa7,0x5f,0xee,0xf4,0x7b,0x52,0x3f,0xc9,0xe9,0x57,
    0x3b,0xfd,0xfe,0xe9,0xb9,0x77,0x45,0xaa,0x7f,0x24,0x28,0x6f,0xcc,0xa3,0x1f,0xe7,
    0xd6,0x9b,0xb1,0xe9,0xc9,0xd8,0x9a,0xed,0x31,0x2e,0xe7,0x0d,0x1b,0xe7,0xce,0x94,
    0x47,0x89,0x8d,0x17,0xd5,0x85,0xec,0x57,0x8c,0xe9,0x03,0xa2,0xbe,0xc6,0x33,0xef,
    0x26,0xbb,0x7c,0x87,0x6c,0x52,0x6c,0x93,0x99,0xef,0x93,0xdc,0x7e,0xfc,0x8a,0x68,
    0xee,0xf9,0xfd,0xf8,0x55,0x21,0xce,0x7b,0x05,0xf6,0xa7,0x88,0xe2,0xe3,0xb9,0x57,
    0xcc,0xd6,0x64,0xb7,0xf7,0xea,0x45,0x79,0x60,0xfe,0x53,0xe8,0x13,0x5c,0xa6,0x39,
    0x2e,0xd0,0x99,0x1a,0xdb,0x34,0x8e,0x9f,0xea,0xb8,0x7c,0xd3,0x71,0xb1,0xbb,0x72,
    0x86,0xe8,0x3e,0x80,0xec,0x15,0x62,0x33,0x45,0xf1,0x7a,0x97,0xc3,0xb3,0x44,0xf1,
    0x41,0x5c,0x17,0xb3,0x3f,0xcd,0xf1,0x9b,0x2d,0xca,0x11,0x9c,0x67,0x91,0x87,0xf9,
    0x9e,0xe3,0x7c,0x5b,0x9c,0xbf,0xc5,0xf9,0x42,0xf6,0x1c,0xfd,0xcc,0x15,0xc5,0x8f,
    0xb8,0xfb,0x6b,0x9e,0xa8,0xed,0x5f,0x50,0x67,0xbe,0xa8,0xde,0x3c,0xce,0xbf,0xc1,
    0xcd,0x1f,0xb2,0x05,0xb1,0x35,0x90,0xdf,0x02,0xd6,0x00,0xa8,0x8f,0x17,0xe6,0xce,
    0x95,0x84,0xb5,0x23,0x6a,0x9e,0x45,0x94,0x4f,0x8c,0xbb,0x1a,0xff,0x53,0x06,0x06,
    0xad,0x83,0x5a,0x43,0x56,0x03,0x04,0xe6,0x0d,0x6a,0xe1,0x41,0x41,0xc7,0x20,0x97,
    0xd6,0x12,0x1b,0x1c,0xf4,0xff,0x02,0x64,0x4f,0x46,0x0c,0x76,0xee,0x0e,0x6a,0x6b,
    0x70,0xc8,0x6c,0x7f,0x91,0x63,0x17,0x49,0x47,0xdb,0x36,0xa6,0x26,0xa8,0x0e,0xec,
    0x3c,0x42,0xec,0x4b,0x41,0x6d,0xd5,0x84,0x8c,0xc3,0x10,0xf2,0x03,0x87,0xc7,0x88,
    0x0d,0x0f,0x8a,0x0f,0x76,0x7a,0xb5,0x41,0xc7,0x27,0x8e,0xff,0x08,0x62,0xe2,0xb0,
    0x91,0xc4,0xa0,0xfa,0x65,0xd6,0x82,0xf7,0x04,0x1d,0x0f,0x7d,0xc8,0x87,0xb3,0x06,
    0xb1,0xf8,0x36,0x48,0x56,0x37,0x5e,0x20,0x1f,0xe4,0x04,0xc6,0x2d,0xc8,0x9d,0xfb,
    0x5a,0x47,0x96,0xa5,0xba,0xed,0xd4,0x45,0xcd,0x31,0x90,0xf7,0x05,0xc6,0x37,0x48,
    0xc7,0xba,0xb4,0x9d,0xf9,0xb6,0x94,0xfe,0x96,0x3a,0x7f,0x7b,0x68,0x23,0xd5,0x89,
    0xcf,0x15,0x92,0xd5,0xa9,0xbb,0x83,0xd6,0xbf,0x90,0xad,0x17,0x95,0x59,0x9d,0xba,
    0x5a,0x54,0x6e,0x75,0xea,0x72,0xca,0x2d,0x87,0x20,0x5f,0x16,0xdb,0x72,0xfa,0xc4,
    0xbb,0xd5,0xa9,0x6b,0x24,0xab,0x53,0x77,0xbb,0x3a,0xf5,0x11,0x51,0x99,0xd5,0xa9,
    0x6b,0x59,0xa7,0x02,0x37,0x9d,0x75,0xa2,0xb8,0xed,0xbd,0xf5,0xe4,0xb1,0xce,0xd5,
    0xa9,0x1b,0x44,0xbf,0x45,0xd4,0xd1,0xa6,0xaf,0x53,0x37,0x8a,0xca,0xad,0x86,0xac,
    0x0b,0x8a,0x6d,0xec,0xa4,0x4e,0x1d,0xcd,0x1a,0xb2,0xce,0xd5,0x90,0xf7,0x05,0xc5,
    0xb1,0x67,0x26,0xb8,0x3b,0x01,0xf8,0x98,0xd8,0x26,0xf0,0xac,0x1d,0xe3,0xea,0xd4,
    0xb1,0xce,0x47,0xbe,0x4e,0x1d,0x47,0x1f,0x63,0x9d,0x8f,0xf1,0x41,0x71,0xac,0x99,
    0xd9,0x9b,0xe0,0xea,0xd4,0x89,0x41,0x79,0x20,0xa6,0xe3,0xe9,0xd7,0xf6,0xf9,0x93,
    0xa2,0x72,0xf0,0x6b,0x74,0xeb,0x01,0xfc,0xa9,0xd8,0x1a,0xb9,0x1e,0x4f,0x31,0x47,
    0x56,0xb0,0xdf,0xe8,0xea,0x89,0x16,0xc9,0xe6,0x6d,0x31,0xfd,0x71,0x2e,0xa6,0x43,
    0x25,0x8b,0xe9,0x16,0x51,0xb9,0xcd,0x77,0x9b,0x28,0xb6,0xc5,0xc5,0xd4,0xce,0xb0,
    0x9f,0x13,0x0f,0x0e,0x7b,0xd6,0xe9,0x1a,0xf6,0xb5,0xa0,0x3c,0x7c,0xbd,0x31,0x25,
    0x28,0xfe,0xac,0xab,0x37,0xa6,0x06,0xc5,0x6b,0x9c,0xde,0xd7,0x83,0xe2,0xbe,0x06,
    0xf9,0x46,0xe8,0xc8,0x7f,0x34,0xf5,0x6c,0x0e,0xd3,0x83,0xea,0x58,0x0d,0x32,0xc3,
    0xd9,0xb8,0x74,0x7e,0x07,0xc5,0xad,0x06,0x59,0xe6,0xd6,0x1f,0xb2,0xd9,0xb1,0xcd,
    0xe1,0x7a,0xcd,0x71,0xb5,0xc4,0x43,0x1c,0x67,0x35,0xc8,0x52,0x37,0x0e,0xb2,0xb9,
    0xb1,0xcd,0xe7,0xb8,0xf9,0x6e,0x5c,0x03,0xc7,0x59,0x0d,0xb2,0xc4,0x8d,0x83,0xec,
    0x61,0x9c,0x81,0x1c,0xb7,0xc8,0xdd,0xd1,0xdf,0x09,0x3a,0x1f,0xab,0x41,0x96,0x50,
    0xe7,0x61,0xa7,0xb3,0x90,0x3a,0x21,0xa7,0xb3,0x24,0x64,0xfb,0xff,0xb5,0xa0,0xb6,
    0x60,0x7b,0x21,0xfd,0xa5,0xe7,0x08,0x75,0xe7,0x3a,0x7b,0xf3,0x68,0xcf,0x6a,0x14,
    0xd3,0x59,0xea,0xec,0xed,0x0f,0x6a,0x13,0x3e,0xe6,0x71,0xde,0xd0,0x5d,0x46,0xdd,
    0xd9,0xce,0xde,0x83,0xb4,0x67,0x35,0x8c,0xe9,0x2c,0x73,0xf6,0xda,0x82,0xda,0x84,
    0x8f,0x07,0x19,0x7f,0x5b,0xbf,0x15,0x2e,0x2f,0xd6,0x11,0x5b,0x19,0x14,0xc7,0x77,
    0xc3,0x97,0x88,0x7d,0x37,0xa8,0x1d,0xc8,0xec,0xee,0xfe,0x5e,0x50,0x7c,0x71,0x92,
    0xdd,0xa9,0xbf,0x14,0xc5,0xb7,0xf3,0xbe,0xdc,0xe9,0xf6,0x16,0x64,0x3b,0x70,0x36,
    0x72,0x2f,0xe1,0x7d,0x03,0xf7,0xd2,0x5a,0xe6,0xf1,0x36,0xee,0x17,0x7c,0x6b,0x5c,
    0x17,0xf4,0x3b,0xa3,0x7d,0x8f,0x58,0x1f,0x54,0x6f,0x15,0xbf,0x51,0x3d,0x1e,0xd4,
    0xd7,0xfd,0x3c,0x07,0xbf,0x1f,0x54,0xc7,0xf6,0x07,0xb0,0x0d,0xc4,0x24,0xbd,0x67,
    0x4a,0xd2,0x6f,0x93,0x1b,0x83,0xea,0x42,0xf6,0xb8,0xb3,0xf7,0x44,0xd0,0x7d,0x37,
    0x9d,0x67,0xfe,0x0f,0x82,0x72,0xd8,0xc8,0xf5,0x7f,0xc2,0xdd,0x73,0x3f,0x0c,0x2a,
    0x4f,0x92,0xec,0xce,0x7f,0x3e,0xb7,0xf7,0xfd,0x9d,0xff,0x3b,0xca,0xed,0xae,0x7b,
    0x51,0xd4,0x86,0x6f,0x57,0xc6,0x1d,0x06,0xd9,0x2e,0x9e,0xfb,0x55,0xfc,0x6e,0x86,
    0xb1,0x2f,0xba,0x7a,0xa1,0x31,0xe8,0x7d,0xb4,0xe7,0x32,0xf5,0x42,0x53,0x50,0x1b,
    0xbe,0x5e,0x68,0x66,0xbd,0xd0,0xe4,0xea,0x85,0x4d,0x41,0x6d,0x35,0xbb,0x7a,0x61,
    0x33,0xc7,0xee,0xba,0x4c,0xbd,0xd0,0x12,0x54,0xa7,0xc9,0xd5,0x0b,0xcf,0x04,0xb5,
    0xd5,0xe2,0xe2,0xf3,0x23,0xf2,0xf3,0xf5,0xc2,0xd6,0xa0,0x78,0xb3,0xd3,0xfb,0x49,
    0xd0,0xf1,0xbe,0x5e,0xd8,0x46,0xcc,0xd7,0x0b,0x3f,0x25,0xe6,0xeb,0x85,0x9f,0x05,
    0x1d,0x0f,0x7d,0xc8,0xb7,0x72,0x9f,0x58,0x7e,0xed,0x74,0xf7,0xf7,0xc7,0xe4,0x83,
    0x7d,0x81,0x71,0x3b,0x72,0xe7,0xfa,0x0a,0xa7,0xfb,0x11,0x75,0x71,0xf6,0x63,0xdc,
    0x4e,0xe9,0x58,0x17,0x2c,0x97,0x8c,0xd7,0x6e,0xc9,0xe6,0xf9,0x3c,0xf7,0xc0,0x4b,
    0xa2,0xf8,0x2e,0x9e,0x47,0xad,0x6e,0x0f,0x40,0xb6,0x07,0x77,0x37,0x6d,0xed,0x11,
    0xfd,0x4e,0x88,0xef,0xa0,0x2f,0x8b,0x7e,0x27,0xb4,0xbc,0xc1,0x1e,0x7b,0x85,0xb8,
    0xd5,0x13,0xaf,0x8a,0x62,0x76,0xb6,0xee,0x73,0xb6,0x21,0xdb,0x1b,0xdb,0x3e,0xda,
    0xde,0xeb,0xee,0x9a,0x36,0xfe,0x57,0xc7,0xf7,0x4f,0xfb,0xb6,0x5a,0x1b,0x3d,0xc0,
    0xef,0x7e,0xc9,0xbe,0x4f,0xd6,0xbb,0xff,0xbe,0xaf,0x51,0x66,0x35,0xf4,0x01,0x51,
    0xcc,0xff,0xb7,0x3f,0xe8,0xbe,0x01,0x98,0xde,0x21,0x51,0x5d,0xc8,0x1e,0x88,0x08,
    0xfc,0xff,0x3e,0x28,0xde,0x26,0xd9,0x59,0x65,0x3c,0xf7,0x31,0xf6,0xd0,0x7b,0x95,
    0xf7,0x3c,0xe2,0x82,0x31,0x7b,0x5d,0xde,0x1f,0x91,0xce,0xf3,0xfe,0x05,0xc6,0xe6,
    0x0d,0xc6,0xfd,0x04,0x63,0x73,0xcc,0xc5,0x06,0xb2,0xa3,0xb1,0x1d,0xa3,0xcf,0xa3,
    0x8c,0x31,0xbe,0x21,0xff,0x83,0xdf,0x8f,0xed,0x9c,0x39,0x29,0xca,0x63,0x66,0xb4,
    0x8b,0x7c,0x78,0x8b,0x3a,0x27,0x1d,0x97,0x77,0x44,0xf1,0xb7,0x2e,0xb3,0x4f,0x4e,
    0x8b,0xea,0x80,0x8f,0xed,0x93,0x33,0xa2,0x73,0x38,0xcd,0x7c,0x32,0x2e,0xc7,0x38,
    0x7f,0xe8,0xec,0x0b,0xaa,0x83,0x98,0x9c,0x21,0x67,0xe3,0x79,0x36,0xc7,0xf3,0xdf,
    0xe4,0x69,0xb9,0xf8,0x1f,0xd1,0xf1,0x7e,0x2f,0xbd,0x47,0xcc,0xef,0xa5,0xf7,0x89,
    0xf9,0xbd,0xf4,0x81,0xe8,0x78,0xe8,0xbf,0xcf,0xdc,0x9d,0x15,0xfd,0xc0,0x27,0xfc,
    0x7c,0x40,0xce,0x96,0xb7,0xad,0xd2,0xf1,0x9b,0x33,0x9e,0xff,0x8d,0x5e,0xee,0x8e,
    0xed,0xff,0xff,0x25,0x76,0xa2,0x7c,0x1e,0x00,0x00
};

// Generated from:
//
// #version 450 core
//
// #extension GL_EXT_samplerless_texture_functions : require
//
// layout(local_size_x = 4, local_size_y = 4, local_size_z = 1)in;
//
// struct TextWidgetData
// {
//     uvec4 coordinates;
//     vec4 color;
//     uvec4 fontSize;
//     uvec4 text[256 / 16];
// };
//
// struct GraphWidgetData
// {
//     uvec4 coordinates;
//     vec4 color;
//     uvec4 valueWidth;
//     uvec4 values[256 / 4];
// };
//
// layout(set = 0, binding = 0, rgba32f)uniform image2D blendOutput;
//
// layout(set = 0, binding = 1)uniform TextWidgets
// {
//     TextWidgetData textWidgetsData[16];
// };
//
// layout(set = 0, binding = 2)uniform GraphWidgets
// {
//     GraphWidgetData graphWidgetsData[16];
// };
//
// layout(set = 0, binding = 3)uniform utexture2D culledWidgets;
// layout(set = 0, binding = 4)uniform texture2DArray font;
//
// layout(push_constant)uniform PushConstants
// {
//     uvec2 outputSize;
//
//     bool rotateXY;
// } params;
//
// bool intersects(const uvec2 imageCoords, const uvec4 widgetCoords)
// {
//     return all(greaterThanEqual(imageCoords, widgetCoords . xy))&&
//            all(lessThan(imageCoords, widgetCoords . zw));
// }
//
// uint getChar(const uint textWidget, const uvec2 coordInWidget, const uint fontGlyphWidth)
// {
//     const uint charIndex = coordInWidget . x / fontGlyphWidth;
//     const uint packIndex = charIndex / 4;
//     const uvec4 arrayItem = textWidgetsData[textWidget]. text[packIndex / 4];
//     const uint col = packIndex % 4;
//
//     uint packedChars;
//     if(col == 0)
//     {
//         packedChars = arrayItem[0];
//     }
//     else if(col == 1)
//     {
//         packedChars = arrayItem[1];
//     }
//     else if(col == 2)
//     {
//         packedChars = arrayItem[2];
//     }
//     else
//     {
//         packedChars = arrayItem[3];
//     }
//
//     const uint shift =(charIndex % 4)* 8;
//
//     return(packedChars >> shift)& 0xFF;
//
// }
//
// float sampleFont(const uint textChar,
//                  const uvec2 coordInWidget,
//                  const uvec2 fontGlyphSize,
//                  const uint fontMip)
// {
//     const uvec2 coordInGlyph = coordInWidget % fontGlyphSize;
//     return texelFetch(font, ivec3(coordInGlyph, textChar), int(fontMip)). x;
// }
//
// uint getValue(const uint graphWidget, const uvec2 coordInWidget, const uint valueWidth)
// {
//     const uint valueIndex = coordInWidget . x / valueWidth . x;
//     const uvec4 arrayItem = graphWidgetsData[graphWidget]. values[valueIndex / 4];
//     const uint col = valueIndex % 4;
//
//     if(col == 0)
//     {
//         return arrayItem[0];
//     }
//     else if(col == 1)
//     {
//         return arrayItem[1];
//     }
//     else if(col == 2)
//     {
//         return arrayItem[2];
//     }
//     else
//     {
//         return arrayItem[3];
//     }
// }
//
// vec4 blend(const vec4 blendedSoFar, const vec4 color)
// {
//
//     return vec4(blendedSoFar . rgb *(1 - color . a)+ color . rgb * color . a,
//                 blendedSoFar . a *(1 - color . a));
// }
//
// void main()
// {
//     uvec2 imageCoords = gl_GlobalInvocationID . xy;
//     if(any(greaterThanEqual(imageCoords, params . outputSize)))
//     {
//         return;
//     }
//
//     vec4 blendedWidgets = vec4(0, 0, 0, 1);
//
//     const uvec2 subgroupWidgets = texelFetch(culledWidgets, ivec2(gl_WorkGroupID . xy), 0). xy;
//     uint textWidgets = subgroupWidgets . x;
//     uint graphWidgets = subgroupWidgets . y;
//
//     while(graphWidgets != 0)
//     {
//         const uint graphWidget = findLSB(graphWidgets);
//         graphWidgets ^= 1 << graphWidget;
//
//         const uvec4 widgetCoords = graphWidgetsData[graphWidget]. coordinates;
//         if(! intersects(imageCoords, widgetCoords))
//         {
//             continue;
//         }
//
//         if(imageCoords . x == widgetCoords . x || imageCoords . y == widgetCoords . y ||
//             imageCoords . x + 1 == widgetCoords . z || imageCoords . y + 1 == widgetCoords . w)
//         {
//
//             blendedWidgets = vec4(0);
//             continue;
//         }
//
//         const uvec2 coordInWidget = imageCoords - widgetCoords . xy;
//         const uint valueWidth = graphWidgetsData[graphWidget]. valueWidth . x;
//
//         const uint value = getValue(graphWidget, coordInWidget, valueWidth);
//
//         vec4 color = vec4(0);
//         const uint widgetHeight = widgetCoords . w - widgetCoords . y;
//
//         bool indicateOverflow = value > widgetHeight && coordInWidget . y + 4 >= widgetHeight
//                 &&((coordInWidget . x ^ coordInWidget . y)& 1)== 0;
//
//         if((widgetHeight - coordInWidget . y)< value && ! indicateOverflow)
//         {
//             color = graphWidgetsData[graphWidget]. color;
//             blendedWidgets = blend(blendedWidgets, color);
//         }
//     }
//
//     while(textWidgets != 0)
//     {
//         const uint textWidget = findLSB(textWidgets);
//         textWidgets ^= 1 << textWidget;
//
//         const uvec4 widgetCoords = textWidgetsData[textWidget]. coordinates;
//         if(! intersects(imageCoords, widgetCoords))
//         {
//             continue;
//         }
//
//         const uvec2 coordInWidget = imageCoords - widgetCoords . xy;
//         const uvec4 fontSizePacked = textWidgetsData[textWidget]. fontSize;
//         const uvec2 fontGlyphSize = fontSizePacked . xy;
//         const uint fontMip = fontSizePacked . z;
//
//         const uint textChar = getChar(textWidget, coordInWidget, fontGlyphSize . x);
//
//         if(textChar < 95)
//         {
//
//             const float sampleValue = sampleFont(textChar, coordInWidget, fontGlyphSize, fontMip);
//
//             vec4 color = vec4(0, 0, 0, 0.4);
//             color = mix(color, textWidgetsData[textWidget]. color, sampleValue);
//
//             blendedWidgets = blend(blendedWidgets, color);
//         }
//     }
//
//     if(blendedWidgets . a < 1)
//     {
//
//         if(params . rotateXY)
//         {
//
//             imageCoords . xy = imageCoords . yx;
//             imageCoords . x = params . outputSize . y - 1 - imageCoords . x;
//         }
//
//         vec3 blendedColor = blendedWidgets . rgb;
//         if(blendedWidgets . a > 0)
//         {
//             const vec4 color = imageLoad(blendOutput, ivec2(imageCoords));
//
//             blendedColor += color . rgb * blendedWidgets . a;
//         }
//
//         imageStore(blendOutput, ivec2(imageCoords), vec4(blendedColor, 1));
//     }
// }