#pragma once

#include "SyntopiaCore/Math/Vector3.h"
#include "Object3D.h"

namespace SyntopiaCore {
	namespace GLEngine {	

		class Dot : public Object3D {
		public:
			Dot(SyntopiaCore::Math::Vector3f pos);

			virtual ~Dot();

			virtual void draw();

		private:
			SyntopiaCore::Math::Vector3f pos;
		};

	}
}