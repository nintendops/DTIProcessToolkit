/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkVectorMaskNegatedImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2008/07/02 15:54:54 $
  Version:   $Revision: 1.4 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkVectorMaskNegatedImageFilter_h
#define __itkVectorMaskNegatedImageFilter_h

#include "itkBinaryFunctorImageFilter.h"
#include "itkNumericTraits.h"

namespace itk
{

/** \class VectorMaskNegatedImageFilter
 * \brief Implements an operator for pixel-wise masking of the input
 * image with the negative of a mask.
 *
 * This class is parametrized over the types of the
 * input image type, the mask image type and the type of the output image.
 * Numeric conversions (castings) are done by the C++ defaults.
 *
 * The pixel type of the input 2 image must have a valid defintion of the
 * operator != with zero . This condition is required because internally this
 * filter will perform the operation
 *
 *        if pixel_from_mask_image != 0
 *             pixel_output_image = 0
 *        else
 *             pixel_output_image = pixel_input_image
 *
 * The pixel from the input 1 is cast to the pixel type of the output image.
 *
 * Note that the input and the mask images must be of the same size.
 *
 * \warning Any pixel value other than 0 will not be masked out.
 *
 * \sa MaskImageFilter
 * \ingroup IntensityImageFilters  Multithreaded
 */
namespace Functor
{

template <class TInput, class TMask, class TOutput>
class VectorMaskNegatedInput
{
public:
  typedef typename NumericTraits<TInput>::AccumulateType AccumulatorType;

  VectorMaskNegatedInput()
  {
  };
  ~VectorMaskNegatedInput()
  {
  };
  bool operator!=( const VectorMaskNegatedInput & ) const
  {
    return false;
  }

  bool operator==( const VectorMaskNegatedInput & other ) const
  {
    return !(*this != other);
  }

  inline TOutput operator()( const TInput & A, const TMask & B)
  {
    if( B != NumericTraits<TMask>::ZeroValue() )
      {
      TInput R = A;
      R.Fill(0);
      return R;
      }
    else
      {
      return static_cast<TOutput>( A );
      }
  }

};

}
template <class TInputImage, class TMaskImage, class TOutputImage>
class ITK_EXPORT VectorMaskNegatedImageFilter :
  public
  BinaryFunctorImageFilter<TInputImage, TMaskImage, TOutputImage,
                           Functor::VectorMaskNegatedInput<
                             typename TInputImage::PixelType,
                             typename TMaskImage::PixelType,
                             typename TOutputImage::PixelType>   >

{
public:
  /** Standard class typedefs. */
  typedef VectorMaskNegatedImageFilter Self;
  typedef BinaryFunctorImageFilter<TInputImage, TMaskImage, TOutputImage,
                                   Functor::VectorMaskNegatedInput<
                                     typename TInputImage::PixelType,
                                     typename TMaskImage::PixelType,
                                     typename TOutputImage::PixelType>
                                   >  Superclass;
  typedef SmartPointer<Self>       Pointer;
  typedef SmartPointer<const Self> ConstPointer;

/** Method for creation through the object factory. */
  itkNewMacro(Self);
protected:
  VectorMaskNegatedImageFilter()
  {
  }

  virtual ~VectorMaskNegatedImageFilter()
  {
  }

private:
  VectorMaskNegatedImageFilter(const Self &); // purposely not implemented
  void operator=(const Self &);               // purposely not implemented

};

} // end namespace itk

#endif
