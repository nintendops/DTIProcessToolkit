#include "tensorscalars.h"

#include <itkMetaDataObject.h>

// Filters
#include <itkTensorFractionalAnisotropyImageFilter.h>
#include <itkShiftScaleImageFilter.h>
#include <itkGradientMagnitudeRecursiveGaussianImageFilter.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

// My ITK Filters
#include "itkVectorMaskNegatedImageFilter.h"
#include "itkTensorMeanDiffusivityImageFilter.h"
#include "itkTensorColorFAImageFilter.h"
#include "itkTensorNegativeEigenValueImageFilter.h"
#include "itkTensorPrincipalEigenvectorImageFilter.h"
#include "itkVectorClosestDotProductImageFilter.h"
#include "itkTensorFAGradientImageFilter.h"
#include "itkTensorRotateImageFilter.h"

// Global constants
const char* NRRD_MEASUREMENT_KEY = "NRRD_measurement frame";

template<>
itk::Image<double, 3>::Pointer createFA<double>(TensorImageType::Pointer timg) // Tensor image
{
  typedef itk::TensorFractionalAnisotropyImageFilter<TensorImageType,RealImageType> FAFilterType;
  FAFilterType::Pointer fafilter = FAFilterType::New();
  fafilter->SetInput(timg);
  fafilter->Update();
    
  return fafilter->GetOutput();
}

template<>
itk::Image<unsigned short, 3>::Pointer createFA<unsigned short>(TensorImageType::Pointer timg)      // Tensor image
{
  RealImageType::Pointer realfa = createFA<double>(timg);
    
  typedef itk::ShiftScaleImageFilter<RealImageType,IntImageType> ShiftScaleFilterType;
  ShiftScaleFilterType::Pointer scalefilter = ShiftScaleFilterType::New();
  scalefilter->SetInput(realfa);
  scalefilter->SetShift(0);
  scalefilter->SetScale(10000);
  scalefilter->Update();

  return scalefilter->GetOutput();
}

template<>
itk::Image<double, 3>::Pointer createMD<double>(TensorImageType::Pointer timg) // Tensor image
{
  typedef itk::TensorMeanDiffusivityImageFilter<TensorImageType,RealImageType> MDFilterType;
  MDFilterType::Pointer mdfilter = MDFilterType::New();
  mdfilter->SetInput(timg);
  mdfilter->Update();
    
  return mdfilter->GetOutput();
}

template<>
itk::Image<unsigned short, 3>::Pointer createMD<unsigned short>(TensorImageType::Pointer timg)      // Tensor image
{
  RealImageType::Pointer realmd = createMD<double>(timg);
    
  typedef itk::ShiftScaleImageFilter<RealImageType,IntImageType> ShiftScaleFilterType;
  ShiftScaleFilterType::Pointer scalefilter = ShiftScaleFilterType::New();
  scalefilter->SetInput(realmd);
  scalefilter->SetShift(0);
  scalefilter->SetScale(100000);
  scalefilter->Update();

  return scalefilter->GetOutput();
}

GradientImageType::Pointer createFAGradient(TensorImageType::Pointer timg, // Tensor image
                                            double sigma)                  // sigma
{
  typedef itk::TensorFAGradientImageFilter<double> FAGradientImageFilter;

  FAGradientImageFilter::Pointer fagradfilter = FAGradientImageFilter::New();
  fagradfilter->SetInput(timg);
  fagradfilter->SetSigma(sigma);

  return fagradfilter->GetOutput();
}

RealImageType::Pointer createFAGradMag(TensorImageType::Pointer timg,      // Tensor image
                                       double sigma)
{
  typedef itk::TensorFractionalAnisotropyImageFilter<TensorImageType,RealImageType> FAFilterType;
  FAFilterType::Pointer fafilter = FAFilterType::New();
  fafilter->SetInput(timg);
    
  // If scale option set scale fa, and write out an integer image


  typedef itk::ShiftScaleImageFilter<RealImageType,RealImageType> ShiftScaleFilterType;
  ShiftScaleFilterType::Pointer scalefilter = ShiftScaleFilterType::New();
  scalefilter->SetInput(fafilter->GetOutput());
  scalefilter->SetShift(0);
  scalefilter->SetScale(10000);

  typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<RealImageType,RealImageType> GradMagFilterType;
  GradMagFilterType::Pointer gradmag = GradMagFilterType::New();
  gradmag->SetInput(scalefilter->GetOutput());
  gradmag->SetSigma(sigma);
  gradmag->SetNormalizeAcrossScale(false);
  gradmag->Update();

  return gradmag->GetOutput();
}


RGBImageType::Pointer createColorFA(TensorImageType::Pointer timg)      // Tensor image
{
  typedef itk::RGBPixel<unsigned char> RGBPixel;
  typedef itk::Image<RGBPixel,3> RGBImageType;
  typedef itk::TensorColorFAImageFilter<TensorImageType,RGBImageType> FAFilterType;
  FAFilterType::Pointer fafilter = FAFilterType::New();
  fafilter->SetInput(timg);
  fafilter->Update();
  
  return fafilter->GetOutput();
}

void createPrincipalEigenvector(TensorImageType::Pointer timg,      // Tensor image
                                const std::string &eigvecname,    // Output
                                                                  // principal
                                                                  // eigenvector name
                                const std::string &dotprodname,   // Ouput
                                                                  // closest
                                                                  // dotproduct
                                                                  // name
                                GradientListType::Pointer gradientlist)
                                
{
    itk::MetaDataDictionary & dict = timg->GetMetaDataDictionary();

    if(dict.HasKey(NRRD_MEASUREMENT_KEY))
      {
      // measurement frame
      vnl_matrix<double> mf(3,3);
      // imaging frame
      vnl_matrix<double> imgf(3,3);
      
      std::vector<std::vector<double> > nrrdmf;
      itk::ExposeMetaData<std::vector<std::vector<double> > >(dict,NRRD_MEASUREMENT_KEY,nrrdmf);
      
      imgf = timg->GetDirection().GetVnlMatrix();
      for(unsigned int i = 0; i < 3; ++i)
        {
        for(unsigned int j = 0; j < 3; ++j)
          {
          mf(i,j) = nrrdmf[i][j];
          
          if(i == j)
            nrrdmf[i][j] = 1.0;
          else
            nrrdmf[i][j] = 0.0;
          }
        }
      
      itk::EncapsulateMetaData<std::vector<std::vector<double> > >(dict,NRRD_MEASUREMENT_KEY,nrrdmf);
      
      typedef itk::TensorRotateImageFilter<TensorImageType, TensorImageType, double> TensorRotateFilterType;
      TensorRotateFilterType::Pointer trotate = TensorRotateFilterType::New();
      trotate->SetInput(timg);
      trotate->SetRotation(vnl_svd<double>(imgf).inverse()*mf);
      trotate->Update();
      timg = trotate->GetOutput();
      
    }

  typedef itk::CovariantVector<double, 3> VectorPixel;
  typedef itk::Image<VectorPixel, 3> VectorImageType;
  typedef itk::TensorPrincipalEigenvectorImageFilter<TensorImageType,VectorImageType> PrincipalEigenvectorFilterType;

  PrincipalEigenvectorFilterType::Pointer ppdfilter = PrincipalEigenvectorFilterType::New();
  ppdfilter->SetInput(timg);

  if(eigvecname != "")
    {
    typedef itk::ImageFileWriter<VectorImageType> VectorImageWriterType;
    VectorImageWriterType::Pointer vectorwriter = VectorImageWriterType::New();
    vectorwriter->SetInput(ppdfilter->GetOutput());
    vectorwriter->SetFileName(eigvecname.c_str());
    try
      {
      vectorwriter->Update();
      }
    catch (itk::ExceptionObject & e)
      {
      std::cerr << "Couldn't write principal eigenvector field" << std::endl;
      std::cerr << e << std::endl;
      }
    }

  if(dotprodname != "")
    {
 
    typedef itk::VectorClosestDotProductImageFilter<VectorImageType, RealImageType> ClosestDotProductFilterType;
    ClosestDotProductFilterType::Pointer cdpfilter = ClosestDotProductFilterType::New();
    cdpfilter->SetInput(ppdfilter->GetOutput());
    cdpfilter->SetGradientList(gradientlist);

    typedef itk::ImageFileWriter<RealImageType> RealImageWriterType;
    RealImageWriterType::Pointer realwriter = RealImageWriterType::New();
    realwriter->SetUseCompression(true);
    realwriter->SetInput(cdpfilter->GetOutput());
    realwriter->SetFileName(dotprodname.c_str());

    try
      {
      realwriter->Update();
      }
    catch (itk::ExceptionObject & e)
      {
      std::cerr << "Couldn't write closest dot product image" << std::endl;
      std::cerr << e << std::endl;
      }
    } // end if(dotprodname != "")
}

LabelImageType::Pointer createNegativeEigenValueLabel(TensorImageType::Pointer timg)
{
  typedef itk::TensorNegativeEigenValueImageFilter<TensorImageType,
    LabelImageType> NegEigLabelFilterType;

  NegEigLabelFilterType::Pointer negeigdetect = NegEigLabelFilterType::New();
  negeigdetect->SetInput(timg);
  negeigdetect->Update();
 
  return negeigdetect->GetOutput();
}

