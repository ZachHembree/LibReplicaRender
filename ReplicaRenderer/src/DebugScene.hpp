#pragma once
#include "D3D11/Effect.hpp"
#include "D3D11/Mesh.hpp"
#include "D3D11/RenderComponent.hpp"

class Replica::InputComponent;

namespace Replica::D3D11
{
	class DebugScene : public RenderComponentBase
	{
	public:
		DebugScene(Renderer& renderer, InputComponent& input);

		void Draw(Context& ctx) override;

	private:
		Effect testEffect;
		UniqueVector<Mesh> primitives;
		InputComponent& input;
	};
}