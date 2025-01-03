#pragma once

enum BlockType {
    BLOCK_AIR,
    GRASS_BLOCK,
    OAK_LOG,
    OAK_LEAVES
};

// 透明方块列表
const BlockType transparentBlocks[] = {
    BLOCK_AIR,
    OAK_LEAVES
};

// 判断方块是否透明
bool isTransparent(BlockType type) {
    for (BlockType transparentBlock : transparentBlocks) {
        if (type == transparentBlock) {
            return true;
        }
    }
    return false;
}