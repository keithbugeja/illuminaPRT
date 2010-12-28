//----------------------------------------------------------------------------------------------
//	Filename:	Matrix.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//	Implementation for a 3x3 Matrix
//----------------------------------------------------------------------------------------------
#pragma once

#include "Maths/Maths.h"
#include "Geometry/Vector3.h"
#include "Geometry/Matrix3x3.h"
#include "Geometry/Matrix4x4.h"

namespace Illumina 
{
	namespace Core
	{
		class Transformation
		{
		protected:
			bool m_hasRotation,
				m_hasTranslation,
				m_hasScaling,
				m_isIdentity;

			Matrix4x4	m_transform;
			Matrix3x3	m_rotation, 
						m_rotationInverse;

			Vector3		m_translation;
			Vector3		m_scaling;


		public:
			Transformation(void);
			Transformation(const Matrix3x3 &p_rotation, const Vector3 &p_scaling, const Vector3 &p_translation);

			void Reset(void);

			Vector3 Apply(const Vector3 &p_vector) const;
			Vector3 ApplyInverse(const Vector3 &p_vector) const;

			Vector3 Scale(const Vector3 &p_vector) const;
			Vector3 ScaleInverse(const Vector3 &p_vector) const;
			Vector3 Rotate(const Vector3 &p_vector) const;
			Vector3 RotateInverse(const Vector3 &p_vector) const;
			Vector3 Translate(const Vector3 &p_vector) const;

			void Apply(const Vector3 &p_vector, Vector3 &p_out) const;
			void ApplyInverse(const Vector3 &p_vector, Vector3 &p_out) const;

			void Scale(const Vector3 &p_vector, Vector3 &p_out) const;
			void ScaleInverse(const Vector3 &p_vector, Vector3 &p_out) const;
			void Rotate(const Vector3 &p_vector, Vector3 &p_out) const;
			void RotateInverse(const Vector3 &p_vector, Vector3 &p_out) const;
			void Translate(const Vector3 &p_vector, Vector3 &p_out) const;

			Matrix3x3 GetRotationInverse(void) const;
			void GetRotationInverse(Matrix3x3 &p_out) const;

			Matrix3x3 GetRotation(void) const;
			void GetRotation(Matrix3x3 &p_out) const;
			void SetRotation(const Matrix3x3 &p_rotation);

			Vector3 GetTranslation(void) const; 
			void GetTranslation(Vector3 &p_out) const;
			void SetTranslation(const Vector3 &p_translation);

			Vector3 GetScaling(void) const;
			void GetScaling(Vector3 &p_out) const;
			void SetScaling(const Vector3 &p_scaling);

			Matrix4x4 GetTransform(void);
			void GetTransform(Matrix4x4 &p_out);

			// inline methods
			inline bool IsIdentity(void) const { return m_isIdentity; }
			inline bool HasScaling(void) const { return m_hasScaling; }
			inline bool HasRotation(void) const { return m_hasRotation; }
			inline bool HasTranslation(void) const { return m_hasTranslation; }

		protected:
			void UpdateTransform(void);
		};
	}
}
