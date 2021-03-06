// Extracted from: ModelView.c++ - an Abstract Base Class for a combined Model and View for OpenGL

#include "ModelView.h"

void ModelView::addToGlobalPan(double dxInLDS, double dyInLDS, double dzInLDS)
{
	// TODO: Delete or comment out the following std::cout statement when
	//       everything else is done here.
		double dxInECMC = (0.5 * dxInLDS) * (last_ecXmax - last_ecXmin);
		double dyInECMC = (0.5 * dyInLDS) * (last_ecYmax - last_ecYmin);

		cryph::AffVector vec(dxInECMC, dyInECMC, dzInLDS); //NOTE: dzInLDS will always be 0
		cryph::Matrix4x4 trans = cryph::Matrix4x4::translation(vec);
		dynamic_view = trans * dynamic_view;

	// TODO: 1. Use last_ecXmin, et al. with dxInLDS, et al. to translate the LDS
	//          pan vector to an EC pan vector.
	//       2. UPDATE dynamic_view
	// TODO: 3. The updated dynamic_view will be used in ModelView::getMatrices
}

void ModelView::addToGlobalRotationDegrees(double rx, double ry, double rz)
{
	// TODO: Delete or comment out the following std::cout statement when
	//       everything else is done here.
	// TODO: 1. UPDATE dynamic_view
	// TODO: 2. The updated dynamic_view will be used in ModelView::getMatrices

	cryph::Matrix4x4 xRotation = cryph::Matrix4x4::xRotationDegrees(rx);
	cryph::Matrix4x4 yRotation = cryph::Matrix4x4::yRotationDegrees(ry);
	cryph::Matrix4x4 zRotation = cryph::Matrix4x4::zRotationDegrees(rz); //NOTE: this will always be 0

	dynamic_view = xRotation * yRotation * zRotation * dynamic_view;
}

void ModelView::getMatrices(cryph::Matrix4x4& mc_ec, cryph::Matrix4x4& ec_lds)
{
	// TODO: Delete or comment out the following std::cout statement when
	//       everything else is done here.
	// 1. Create the mc_ec matrix:
	//    Create a local variable of type Matrix4x4 called M_ECu from the eye,
	//    center, and up. Recall that those values are initialized in the main
	//    program and may be modified at any time while processing interactive
	//    events. Their current values are stored in protected class variables
	//    that you directly access.
	//
	cryph::Matrix4x4 M_ECu = cryph::Matrix4x4::lookAt(ModelView::eye,
			ModelView::center, ModelView::up);

	cryph::AffVector vec(0, 0, ModelView::distEyeCenter);
	cryph::Matrix4x4 preTrans = cryph::Matrix4x4::translation(vec);
	cryph::Matrix4x4 postTrans = cryph::Matrix4x4::translation(-vec);

	mc_ec = postTrans * dynamic_view * preTrans * M_ECu;

	//    For project 3: Either:
	//        mc_ec = dynamic_view * M_ECu (if rotations are to be about the eye)
	//    or:
	//        mc_ec = postTrans * dynamic_view * preTrans * M_ECu (if
	//                         rotations are to be about the center of attention)
	//    NOTE: If using preTrans/postTrans, be sure you build them CORRECTLY here!!!

	// 2. Create the ec_lds matrix:
	//    Use the mcRegionOfInterest. (As with eye, center, up, the mcRegionOfInterest
	//    is initialized in main and is subject to modification at any time while
	//    processing events. Its current values are also stored in protected class
	//    variables that you directly access.

	//    2.a. Determine the maximum of delta_mcX, delta_mcY, and delta_mcZ. (For
	//         example, delta_mcX = mcRegionOfInterest[1] - mcRegionOfInterest[0].)
	//         Suppose you store the maximum of these delta_mc* values in "maxDelta".


	double deltaMCs[3];
	double* xyz = ModelView::mcRegionOfInterest;

	deltaMCs[0] = xyz[1] - xyz[0];
	deltaMCs[1] = xyz[3] - xyz[2];
	deltaMCs[2] = xyz[5] - xyz[4];

	double xmid = (xyz[0] + xyz[1]) / 2;
	double ymid = (xyz[2] + xyz[3]) / 2;
	double zmid = (xyz[4] + xyz[5]) / 2;

	double maxDelta = deltaMCs[0]; // TODO: compute this as just described.
	for(int i=1; i<3; i++)
	{
		if (deltaMCs[i] > maxDelta)
			maxDelta = deltaMCs[i];
	}

	double halfWidth = 0.5 * maxDelta;

	//    2.b. In project 3 & 4: Scale "halfWidth" by "dynamic_zoomScale"
	//    2.c. initialize the XY direction of the view volume as:
	//double sphereRad = sqrt( pow(xyz[1] - xyz[0], 2) + pow(xyz[3] - xyz[2], 2) + pow(xyz[5]-xyz[4], 2)) * dynamic_zoomScale;

	// last_ecXmin = xmid - sphereRad;
	// last_ecXmax = xmid + sphereRad;
	// last_ecYmin = ymid - sphereRad;
	// last_ecYmax = ymid + sphereRad;

	last_ecXmin = last_ecYmin = -halfWidth * dynamic_zoomScale;
	last_ecXmax = last_ecYmax =  halfWidth * dynamic_zoomScale;

	double vAR = Controller::getCurrentController() -> getViewportAspectRatio();
	ModelView::matchAspectRatio(last_ecXmin, last_ecXmax, last_ecYmin, last_ecYmax, vAR);

	if (ModelView::projType == ORTHOGONAL)
		ec_lds = cryph::Matrix4x4::orthogonal(last_ecXmin, last_ecXmax, last_ecYmin, last_ecYmax,
				ModelView::ecZmin, ModelView::ecZmax);
	else if (ModelView::projType == PERSPECTIVE)
		ec_lds = cryph::Matrix4x4::perspective(ModelView::ecZpp,
				last_ecXmin, last_ecXmax, last_ecYmin, last_ecYmax, ModelView::ecZmin, ModelView::ecZmax);
	else // Must be OBLIQUE
		ec_lds = cryph::Matrix4x4::oblique(ModelView::ecZpp,
				last_ecXmin, last_ecXmax, last_ecYmin, last_ecYmax, ModelView::ecZmin, ModelView::ecZmax,
				ModelView::obliqueProjectionDir);

	// THEN IN THE CALLER OF THIS METHOD:
	//
	// float mat[16];
	// glUniformMatrix4fv(shaderIF->ppuLoc("mc_ec"), 1, false, mc_ec.extractColMajor(mat));
	// glUniformMatrix4fv(shaderIF->ppuLoc("ec_lds"), 1, false, ec_lds.extractColMajor(mat));
	//
	// (The extractColMajor method copies the elements of the matrix into the given
	// array which is assumed to be of length 16. It then returns the array pointer
	// so it can be used as indicated in the two calls. Since the array is immediately
	// copied by glUniformMatrix to the GPU, "mat" can be reused as indicated.)
}

void ModelView::scaleGlobalZoom(double multiplier)
{
	if(multiplier !=0)
	{
		dynamic_zoomScale *= multiplier;
	}
}
