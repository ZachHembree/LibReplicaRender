#pragma once
#include "MinWindow.hpp"
#include <imgui.h>
#include <imfilebrowser.h>
#include "D3D11/RenderComponent.hpp"
#include "D3D11/Renderer.hpp"
#include "D3D11/Effect.hpp"
#include "D3D11/Resources/Texture2D.hpp"
#include "D3D11/Resources/Sampler.hpp"
#include "D3D11/Mesh.hpp"
#include "D3D11/Primitives.hpp"
#include "D3D11/Shaders/BuiltInShaders.hpp"
#include <DirectXTex.h>

using DirectX::ScratchImage;

namespace Replica::D3D11
{
	class DebugImageViewer : public RenderComponentBase
	{
	public:
		DebugImageViewer(Renderer& renderer) : 
            RenderComponentBase(renderer),
            samp(renderer.GetDevice(), TexFilterMode::LINEAR, TexClampMode::BORDER),
            texQuadEffect(renderer.GetDevice(), g_PosTextured2DEffect)
        {
            fileDialog.SetTitle("Load Image");
            fileDialog.SetTypeFilters({".bmp", ".gif", ".ico", "jpg", ".png", ".tiff", ".tif", ".dds"});

            MeshDef quadDef = Primitives::GeneratePlane<VertexPos2D>(ivec2(0), 2.0f);
            quad = Mesh(renderer.GetDevice(), quadDef);
        }

        void DrawUI(Context& ctx)
        {
            if (ImGui::Begin("Image Viewer Debug"))
            {
                if (ImGui::Button("Load Image"))
                    fileDialog.Open();

                if (ImGui::Button("Enter Fullscreen"))
                {
                    MinWindow& win = GetWindow();
                    ivec2 mres = win.GetMonitorResolution();
                    win.DisableStyleFlags(WndStyle(g_DefaultWndStyle, 0));
                    win.SetBodySize(mres);
                    win.SetPos(ivec2(0));
                }

                if (ImGui::Button("Exit Fullscreen"))
                {
                    MinWindow& win = GetWindow();
                    win.EnableStyleFlags(WndStyle(g_DefaultWndStyle, 0));
                    win.SetBodySize(vec2(1280, 800));
                }
            }
            ImGui::End();

            fileDialog.Display();

            if (fileDialog.HasSelected())
            {
                if (tex.GetIsValid())
                    tex.UpdateTextureWIC(ctx, fileDialog.GetSelected().native(), buffer);
                else
                    tex = Texture2D::FromImageWIC(GetDevice(), fileDialog.GetSelected().native());

                fileDialog.ClearSelected();
            }
        }

        void Update(Context& ctx) override
        {
            DrawUI(ctx);

            if (tex.GetIsValid())
            { 
                texQuadEffect.SetTexture(L"tex", tex);

                const vec2 bodySize = GetWindow().GetBodySize(),
                    imgSize = tex.GetSize();
                vec2 vpSize = bodySize;
                const float wndAspect = bodySize.x / bodySize.y,
                    imgAspect = imgSize.x / imgSize.y;

                if (wndAspect >= imgAspect)
                    vpSize.x = bodySize.y * imgAspect;
                else
                    vpSize.y = bodySize.x / imgAspect;

                ctx.RSSetViewport(vpSize, 0.5f * (bodySize - vpSize));
            }

            texQuadEffect.SetSampler(L"samp", samp);
            texQuadEffect.Update(ctx);
            quad.Update(ctx);
        }

		void Draw(Context& ctx) override 
		{
            quad.Draw(ctx);
		}

	private:
        ScratchImage buffer;
		ImGui::FileBrowser fileDialog;
        Texture2D tex;
        Sampler samp;
        Effect texQuadEffect;
        Mesh quad;
	};
}