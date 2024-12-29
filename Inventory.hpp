#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h" 
#include <vector>
#include <string>
#include "Block.hpp"
#include "TextureManager.hpp"

class Inventory {
private:
    static const int INVENTORY_SIZE = 9; // 物品栏大小
    std::vector<BlockType> slots; // 存储每个槽位的物品ID
    int selectedSlot; // 当前选中的槽位
    std::vector<GLuint> textureViews;
public:
    Inventory(const TextureManager& textureManager) : selectedSlot(0) {
        slots.resize(INVENTORY_SIZE, BLOCK_AIR); // 初始化所有槽位为空(0)
        slots[0] = BlockType::GRASS_BLOCK; // 第一个槽位放置草方块
        slots[1] = BlockType::OAK_LOG; // 第二个槽位放置圆木方块
        slots[2] = BlockType::OAK_LEAVES; // 第三个槽位放置树叶方块
        
        createTextureViews(textureManager);
        
    }

    void render() {
        // 获取窗口大小
        ImGuiIO& io = ImGui::GetIO();
        float windowWidth = io.DisplaySize.x;
        float windowHeight = io.DisplaySize.y;

        // 计算物品栏位置
        float inventoryWidth = 400; // 物品栏宽度
        float inventoryHeight = 100; // 物品栏高度
        float padding = 20; // 底部边距

        ImVec2 inventoryPos(
            (windowWidth - inventoryWidth) * 0.5f,  // 水平居中
            windowHeight - inventoryHeight - padding // 底部对齐
        );

        // 设置窗口位置和大小
        ImGui::SetNextWindowPos(inventoryPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(inventoryWidth, inventoryHeight));
        ImGui::Begin("Inventory", nullptr, 
            ImGuiWindowFlags_NoTitleBar | 
            ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoMove);
        
         
        // 渲染物品栏槽位
        for (int i = 0; i < INVENTORY_SIZE; i++) {
            if (i > 0) ImGui::SameLine();
            
            // 如果是选中的槽位,改变背景颜色
            if (i == selectedSlot) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
            }
            // 槽位标签
            std::string label = "##slot" + std::to_string(i);
            // 材质索引
            ImTextureID textureIndex = static_cast<intptr_t>(textureViews[(getTextureLayer(slots[i]))]);
            // 创建按钮代表槽位
            ImGui::ImageButton(label.c_str(),
                textureIndex, 
            ImVec2(40, 40)
            );

            if (i == selectedSlot) {
                ImGui::PopStyleColor();
            }
        }

        ImGui::End();
    }

    // 通过鼠标滚轮切换选中的槽位
    void scrollSlot(double yOffset) {
        selectedSlot -= static_cast<int>(yOffset);
        if (selectedSlot < 0) selectedSlot = INVENTORY_SIZE - 1;
        if (selectedSlot >= INVENTORY_SIZE) selectedSlot = 0;
    }

    void createTextureViews(const TextureManager& textureManager) {
        textureViews.resize(textureManager.texture_imgs.size() + 1);
        // 第 0 层是空气
        // 这里再创建一次材质，因为还没有找到能让imgui使用opengl纹理数组的方法
        for(int i = 1;i <= textureManager.texture_imgs.size();i++){
            // 注意 texture_imgs 索引要减1
            textureViews[i] = TextureManager::loadTexture(textureManager.texture_imgs[i - 1].c_str());
        }
    }

    // 获取当前选中槽位的物品ID
    BlockType getSelectedBlock() const {
        return slots[selectedSlot];
    }

};