#pragma once

enum BlockType {
    BLOCK_AIR, 
    GRASS_BLOCK, 
    OAK_LOG,     
    OAK_LEAVES,  
    DIRT_BLOCK,  
    STONE_BLOCK, 
    SAND_BLOCK,
    GLASS_BLOCK,
    OAK_PLANKS,
    STONE_BRICKS
};

// 透明方块列表
const BlockType transparentBlocks[] = {
    BLOCK_AIR,
    OAK_LEAVES, 
    GLASS_BLOCK
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