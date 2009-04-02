/*
    Copyright (c) 2008 NetAllied Systems GmbH

    This file is part of COLLADAFramework.

    Licensed under the MIT Open Source License, 
    for details please see LICENSE file or the website
    http://www.opensource.org/licenses/mit-license.php
*/

#ifndef __COLLADAFW_ANIMATIONCURVE_H__
#define __COLLADAFW_ANIMATIONCURVE_H__

#include "COLLADAFWPrerequisites.h"
#include "COLLADAFWAnimation.h"
#include "COLLADAFWArrayPrimitiveType.h"
#include "COLLADAFWTypes.h"
#include "COLLADAFWFloatOrDoubleArray.h"

namespace COLLADAFW
{

	/** A key frame is a two-dimensional (2D) sampling of data. The first dimension is called the input and is usually
	time, but can be any other real value. The second dimension is called the output and represents the value
	being animated. Using a set of key frames and an interpolation algorithm, intermediate values are
	computed for times between the key frames, producing a set of output values over the interval between the
	key frames. The set of key frames and the interpolation between them define a 2D function called an
	animation curve or function curve. */
	class AnimationCurve : public Animation 
	{
	public:
		enum InterpolationType
		{
			INTERPOLATION_UNKNOWN,
			INTERPOLATION_LINEAR, 
			INTERPOLATION_BEZIER, 
			INTERPOLATION_CARDINAL, 
			INTERPOLATION_HERMITE,
			INTERPOLATION_BSPLINE, 
			INTERPOLATION_STEP,
			INTERPOLATION_MIXED,
		};

		typedef ArrayPrimitiveType<InterpolationType> InterpolationTypeArray;

	private:
		/** The dimension of the output, e.g. 1 for a single float, 3 for a position.*/
		size_t mOutDimension;

		/** The interpolation type of the curve. If the curve uses only one type of interpolation,
		this member is set to this type. If more than one type is used, it is set to INTERPOLATION_MIXED. 
		The mInterpolations array than defines the interpolation between the keys.*/
		InterpolationType mInterpolationType;

		/** The input values of the animation. */
		FloatOrDoubleArray mInputValues;

		/** The output values of the animation. mOutDimension specifies how many of these floats represent one 
		output value. Therefore the size of the array must be the mInputValues.getCount() * mOutDimension. */
		FloatOrDoubleArray mOutputValues;

		/** If mInterpolationType == INTERPOLATION_MIXED, this array defines how the values between the keys
		should be interpolated. The first value defines the interpolation between the first and second key, and so 
		on. Therefore the size of the array must be the mInputValues.getCount(). For other values of mInterpolationType the interpolation is defined by mInterpolationType and this
		array is empty.*/
		InterpolationTypeArray mInterpolations;

		// TODO think about the storage of tangents
		FloatOrDoubleArray mInTangentsValues;
		FloatOrDoubleArray mOutTangentsValues;

	public:

        /** Constructor. */
		AnimationCurve( ObjectId objectId )
			: Animation(objectId, Animation::ANIMATION_CURVE)
			, mOutDimension(0)
			, mInterpolationType(INTERPOLATION_UNKNOWN)
			, mInterpolations(InterpolationTypeArray::OWNER)
		{}

        /** Destructor. */
		virtual ~AnimationCurve(){}


		/** Returns the dimension of the output, e.g. 1 for a single float, 3 for a position.*/
		size_t getOutDimension() const { return mOutDimension; }

		/** Sets the dimension of the output, e.g. 1 for a single float, 3 for a position.*/
		void setOutDimension(size_t outputDimension) { mOutDimension = outputDimension; }

		/** Returns the interpolation type of the curve. If the curve uses only one type of interpolation,
		this member is set to this type. If more than one type is used, it is set to INTERPOLATION_MIXED.*/
		InterpolationType getInterpolationType() const { return mInterpolationType; }

		/** Returns the interpolation type of the curve. If the curve uses only one type of interpolation,
		this member is set to this type. If more than one type is used, it is set to INTERPOLATION_MIXED.*/
		void setInterpolationType(InterpolationType interpolationType) { mInterpolationType = interpolationType; }

		/** Returns the input values of the animation. */
		FloatOrDoubleArray& getInputValues() { return mInputValues; }

		/** Returns the input values of the animation. */
		const FloatOrDoubleArray& getInputValues() const { return mInputValues; }

		/** Returns the output values of the animation. */
		FloatOrDoubleArray& getOutputValues() { return mOutputValues; }

		/** Returns the output values of the animation. */
		const FloatOrDoubleArray& getOutputValues() const { return mOutputValues; }


	private:

        /** Disable default copy ctor. */
		AnimationCurve( const AnimationCurve& pre );

        /** Disable default assignment operator. */
		const AnimationCurve& operator= ( const AnimationCurve& pre );

	};

} // namespace COLLADAFW

#endif // __COLLADAFW_ANIMATIONCURVE_H__