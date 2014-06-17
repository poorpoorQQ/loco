/*
 * BaseControlDynamicGait.cpp
 *
 *  Created on: Feb 24, 2014
 *      Author: gech
 */

#include "loco/torso_control/TorsoControlDynamicGaitPerceptedTerrain.hpp"
#include "loco/common/TerrainModelPerceptedPlane.hpp"

namespace loco {

TorsoControlDynamicGaitPerceptedTerrain::TorsoControlDynamicGaitPerceptedTerrain(LegGroup* legs, TorsoBase* torso,  loco::TerrainModelBase* terrain):
  TorsoControlBase(),
  legs_(legs),
  torso_(torso),
  terrain_(terrain),
  comControl_(legs),
  headingDistanceFromForeToHindInBaseFrame_(0.0),
  terrainSlopeAngle_(0.0),
  terrainRollAngle_(0.0)
{

  std::vector<double> tValues, xValues;
  const double defaultHeight = 0.42;
  desiredTorsoForeHeightAboveGroundInWorldFrameOffset_ = defaultHeight;
  desiredTorsoHindHeightAboveGroundInWorldFrameOffset_ = defaultHeight;
  tValues.push_back(0.00); xValues.push_back(0.0);
  tValues.push_back(0.25); xValues.push_back(0.0);
  tValues.push_back(0.50); xValues.push_back(0.0);
  tValues.push_back(0.75); xValues.push_back(0.0);
  tValues.push_back(1.00); xValues.push_back(0.0);
  desiredTorsoForeHeightAboveGroundInWorldFrame_.setRBFData(tValues, xValues);
  desiredTorsoHindHeightAboveGroundInWorldFrame_.setRBFData(tValues, xValues);
}

TorsoControlDynamicGaitPerceptedTerrain::~TorsoControlDynamicGaitPerceptedTerrain() {

}
bool TorsoControlDynamicGaitPerceptedTerrain::initialize(double dt) {
  const Position foreHipPosition = legs_->getLeg(0)->getWorldToHipPositionInBaseFrame();
  const Position hindHipPosition = legs_->getLeg(2)->getWorldToHipPositionInBaseFrame();
  headingDistanceFromForeToHindInBaseFrame_ = foreHipPosition.x()-hindHipPosition.x();
//  std::cout << "head dist: " << headingDistanceFromForeToHindInBaseFrame_ << std::endl;

  return true;
}



void TorsoControlDynamicGaitPerceptedTerrain::advance(double dt) {
  comControl_.advance(dt);

  const RotationQuaternion orientationWorldToHeading = torso_->getMeasuredState().getWorldToHeadingOrientation();



  Position lateralAndHeadingPositionInWorldFrame = comControl_.getDesiredWorldToCoMPositionInWorldFrame();

  const double desiredForeHeightAboveGroundInWorldFrame = desiredTorsoForeHeightAboveGroundInWorldFrameOffset_+desiredTorsoForeHeightAboveGroundInWorldFrame_.evaluate(torso_->getStridePhase());
  const double desiredHindHeightAboveGroundInWorldFrame = desiredTorsoHindHeightAboveGroundInWorldFrameOffset_+desiredTorsoHindHeightAboveGroundInWorldFrame_.evaluate(torso_->getStridePhase());
  const double desiredMiddleHeightAboveGroundInWorldFrame = (desiredForeHeightAboveGroundInWorldFrame + desiredHindHeightAboveGroundInWorldFrame)/2.0;
  Position desiredLateralAndHeadingPositionInWorldFrame = lateralAndHeadingPositionInWorldFrame;
  Position groundHeightInWorldFrame = desiredLateralAndHeadingPositionInWorldFrame;
  terrain_->getHeight(groundHeightInWorldFrame);
  Position desiredTorsoPositionInWorldFrame(desiredLateralAndHeadingPositionInWorldFrame.x(), desiredLateralAndHeadingPositionInWorldFrame.y(), desiredMiddleHeightAboveGroundInWorldFrame+groundHeightInWorldFrame.z());
//  Position desiredTorsoPositionInWorldFrame(0.0, desiredLateralAndHeadingPositionInWorldFrame.y(), desiredMiddleHeightAboveGroundInWorldFrame+groundHeightInWorldFrame.z());

  /* --- desired orientation --- */

  // pitch angle
  double height = desiredHindHeightAboveGroundInWorldFrame-desiredForeHeightAboveGroundInWorldFrame;
  double pitchAngle = atan2(height,headingDistanceFromForeToHindInBaseFrame_);

  double dampingFactor=1.0;
    terrainSlopeAngle_=terrainSlopeAngle_+dampingFactor*(((TerrainModelPerceptedPlane*) terrain_)->getSlopeAngle()-terrainSlopeAngle_);
    terrainRollAngle_=terrainRollAngle_+dampingFactor*(((TerrainModelPerceptedPlane*) terrain_)->getRollAngle()-terrainRollAngle_);


//    std::cout << "terrain roll angle = " << terrainRollAngle_*180/M_PI << std::endl;





    RotationQuaternion orientationDesiredHeadingToBase = RotationQuaternion(EulerAnglesZyx(0, pitchAngle-terrainSlopeAngle_*3/4, -terrainRollAngle_*0/4));
//  RotationQuaternion orientationDesiredHeadingToBase = RotationQuaternion(AngleAxis(pitchAngle, 0.0, 1.0, 0.0));

  const Position positionForeFeetMidPointInWorldFrame = (legs_->getLeftForeLeg()->getWorldToFootPositionInWorldFrame() + legs_->getRightForeLeg()->getWorldToFootPositionInWorldFrame())/0.5;
  const Position positionHindFeetMidPointInWorldFrame = (legs_->getLeftHindLeg()->getWorldToFootPositionInWorldFrame() + legs_->getRightHindLeg()->getWorldToFootPositionInWorldFrame())/0.5;
  Position positionWorldToDesiredForeFeetMidPointInWorldFrame = positionForeFeetMidPointInWorldFrame+ comControl_.getPositionErrorVectorInWorldFrame();
  Position positionWorldToDesiredHindFeetMidPointInWorldFrame = positionHindFeetMidPointInWorldFrame+ comControl_.getPositionErrorVectorInWorldFrame();

  Vector desiredHeadingDirectionInWorldFrame = Vector(positionWorldToDesiredForeFeetMidPointInWorldFrame-positionWorldToDesiredHindFeetMidPointInWorldFrame);
  desiredHeadingDirectionInWorldFrame.z() = 0.0;

  const Position positionForeHipsMidPointInWorldFrame = (legs_->getLeftForeLeg()->getWorldToHipPositionInWorldFrame() + legs_->getRightForeLeg()->getWorldToHipPositionInWorldFrame())/0.5;
  const Position positionHindHipsMidPointInWorldFrame = (legs_->getLeftHindLeg()->getWorldToHipPositionInWorldFrame() + legs_->getRightHindLeg()->getWorldToHipPositionInWorldFrame())/0.5;


  Vector currentHeadingDirectionInWorldFrame = Vector(positionForeHipsMidPointInWorldFrame-positionHindHipsMidPointInWorldFrame);
  currentHeadingDirectionInWorldFrame.z() = 0.0;

  RotationQuaternion orientationHeadingToDesiredHeading;
  orientationHeadingToDesiredHeading.setFromVectors(currentHeadingDirectionInWorldFrame.toImplementation(),desiredHeadingDirectionInWorldFrame.toImplementation());




  RotationQuaternion desOrientationWorldToBase = orientationDesiredHeadingToBase*orientationHeadingToDesiredHeading*orientationWorldToHeading;

  /* --- end desired orientation --- */

  torso_->getDesiredState().setWorldToBasePoseInWorldFrame(Pose(desiredTorsoPositionInWorldFrame, desOrientationWorldToBase));
//  torso_->getDesiredState().setBaseTwistInBaseFrame(Twist(desiredLinearVelocity, desiredAngularVelocity));


  /* if a stance leg lost contact, lower it to re-gain contact */
  for (auto leg : *legs_) {
    if (leg->isInStanceMode()) {
      Position positionWorldToFootInWorldFrame =  leg->getWorldToFootPositionInWorldFrame();


      if (!leg->isGrounded()) {
        positionWorldToFootInWorldFrame.z() -= 0.01;
      }
      const Position positionWorldToBaseInWorldFrame = torso_->getMeasuredState().getWorldToBasePositionInWorldFrame();
      const Position positionBaseToFootInWorldFrame = positionWorldToFootInWorldFrame - positionWorldToBaseInWorldFrame;
      const Position positionBaseToFootInBaseFrame = torso_->getMeasuredState().getWorldToBaseOrientationInWorldFrame().rotate(positionBaseToFootInWorldFrame);
      leg->setDesiredJointPositions(leg->getJointPositionsFromBaseToFootPositionInBaseFrame(positionBaseToFootInBaseFrame));
    }
  }
}



inline double safeACOS(double val){
  if (val<-1)
    return M_PI;
  if (val>1)
    return 0;
  return acos(val);
}

/**
  Assume that the current quaternion represents the relative orientation between two coordinate frames A and B.
  This method decomposes the current relative rotation into a twist of frame B around the axis v passed in as a
  parameter, and another more arbitrary rotation.

  AqB = AqT * TqB, where T is a frame that is obtained by rotating frame B around the axis v by the angle
  that is returned by this function.

  In the T coordinate frame, v is the same as in B, and AqT is a rotation that aligns v from A to that
  from T.

  It is assumed that vB is a unit vector!! This method returns TqB, which represents a twist about
  the axis vB.
*/
RotationQuaternion TorsoControlDynamicGaitPerceptedTerrain::decomposeRotation(const RotationQuaternion& AqB, const Vector& vB) {


  const Vector vA =  AqB.inverseRotate(vB).normalized();

  Vector rotAxis = (vA.cross(vB).normalized());

  if (rotAxis.norm() == 0) {
    rotAxis = Vector::UnitZ();
  }
  rotAxis *= -1.0;
  double rotAngle = -safeACOS(vA.dot(vB));
  const RotationQuaternion TqA = RotationQuaternion(AngleAxis(rotAngle, rotAxis.toImplementation()));
  return AqB*TqA; // TqB

}

RotationQuaternion TorsoControlDynamicGaitPerceptedTerrain::computeHeading(const RotationQuaternion& rquat, const Vector& axis) {
  return decomposeRotation(rquat.conjugated(),axis).conjugated();

}


CoMOverSupportPolygonControl* TorsoControlDynamicGaitPerceptedTerrain::getCoMControl() {
  return &comControl_;
}

bool TorsoControlDynamicGaitPerceptedTerrain::loadParameters(const TiXmlHandle& handle) {
  TiXmlHandle hDynGait(handle.FirstChild("TorsoControl").FirstChild("DynamicGait"));
  if (!comControl_.loadParameters(hDynGait)) {
    return false;
  }
  if (!loadParametersHipConfiguration(hDynGait)) {
    return false;
  }

  return true;
}



bool TorsoControlDynamicGaitPerceptedTerrain::loadParametersHipConfiguration(const TiXmlHandle &hParameterSet)
{


  int iKnot;
  double t, value;
  TiXmlElement* pElem;
  std::string legFrame;

  /* Swing foot configuration*/
  pElem = hParameterSet.FirstChild("HipConfiguration").Element();
  if (!pElem) {
    printf("Could not find HipConfiguration\n");
    return false;
  }

  /**************************************************************************
   * HEIGHT
   ***************************************************************************/

  /* offset */
  pElem = hParameterSet.FirstChild("HipConfiguration").Element();
  if (!pElem) {
    printf("Could not find HipConfiguration!\n");
    return false;
  }
  TiXmlElement* child = hParameterSet.FirstChild("HipConfiguration").FirstChild().ToElement();
       for( child; child; child=child->NextSiblingElement() ){
          if (child->ValueStr().compare("HeightTrajectory") == 0) {
            bool isFore = false;
            bool isHind = false;
            double offset = 0.0;
            if (child->QueryDoubleAttribute("offset", &offset)!=TIXML_SUCCESS) {
              printf("Could not find offset!\n");
            }
            if (child->QueryBoolAttribute("fore", &isFore)==TIXML_SUCCESS) {
              if (isFore) {
                desiredTorsoForeHeightAboveGroundInWorldFrameOffset_ = offset;
                TiXmlHandle hTrajectory(child);
                if(!loadHeightTrajectory(hTrajectory,  desiredTorsoForeHeightAboveGroundInWorldFrame_)) {
                  return false;
                }
              }
            }
            if (child->QueryBoolAttribute("hind", &isHind)==TIXML_SUCCESS) {
              if (isHind) {
                desiredTorsoHindHeightAboveGroundInWorldFrameOffset_ = offset;
                TiXmlHandle hTrajectory(child);
                if(!loadHeightTrajectory(hTrajectory,  desiredTorsoHindHeightAboveGroundInWorldFrame_)) {
                  return false;
                }
              }
            }
          }

       }
//
//      /* front leg frame */
//      pElem = hParameterSet.FirstChild("HipConfiguration").FirstChild("Fore").Element();
//      if (!pElem) {
//        printf("Could not find HipConfiguration:Fore!\n");
//        return false;
//      }
//      TiXmlHandle hTrajectory (hParameterSet.FirstChild("HipConfiguration").FirstChild("Fore").FirstChild("HeightTrajectory"));
//      pElem = hTrajectory.Element();
//      if (!pElem) {
//        printf("Could not find HeightTrajectory!\n");
//        return false;
//      }
//      if (pElem->QueryDoubleAttribute("offset", &desiredFrameHeightOffset)!=TIXML_SUCCESS) {
//        printf("Could not find HeightTrajectory:offset!\n");
//        return false;
//      }
//      if (!loadHeightTrajectory(hTrajectory)) {
//        printf("problem\n");
//        return false;
//      }
//
//
//      /* hind leg frame */
//      pElem = hParameterSet.FirstChild("HipConfiguration").FirstChild("Hind").Element();
//      if (!pElem) {
//        printf("Could not find HipConfiguration:Hind!\n");
//        return false;
//      }
//      TiXmlHandle hTrajectory (hParameterSet.FirstChild("HipConfiguration").FirstChild("Hind").FirstChild("HeightTrajectory"));
//      pElem = hTrajectory.Element();
//      if (!pElem) {
//        printf("Could not find HeightTrajectory!\n");
//        return false;
//      }
//      if (pElem->QueryDoubleAttribute("offset", &desiredFrameHeightOffset)!=TIXML_SUCCESS) {
//        printf("Could not find HeightTrajectory:offset!\n");
//        return false;
//      }
//      if (!loadHeightTrajectory(hTrajectory)) {
//        printf("problem\n");
//        return false;
//      }


//  else {
//
//    // both leg frames
//    pElem = hParameterSet.FirstChild("HipConfiguration").FirstChild("ForeAndHind").FirstChild("HeightTrajectory").Element();
//    if (!pElem) {
//      pElem = hParameterSet.FirstChild("HipConfiguration").FirstChild("FrontAndHind").FirstChild("PitchTrajectory").Element();
//      if (!pElem) {
//        printf("Could not find HeightTrajectory or PitchTrajectory!\n");
//        return false;
//      }
//      TiXmlHandle hTrajectory (hParameterSet.FirstChild("HipConfiguration").FirstChild("FrontAndHind").FirstChild("PitchTrajectory"));
//      if (pElem->QueryDoubleAttribute("offset", &desiredFrameHeightOffset)!=TIXML_SUCCESS) {
//        printf("Could not find PitchTrajectory:offset!\n");
//        return false;
//      }
//      printf("ERRLOR: pitch trajectory not implemented!\n");
//      return false;
//
//    }
//    else {
//
//      TiXmlHandle hTrajectory (hParameterSet.FirstChild("HipConfiguration").FirstChild("FrontAndHind").FirstChild("HeightTrajectory"));
//      if (pElem->QueryDoubleAttribute("offset", &desiredFrameHeightOffset)!=TIXML_SUCCESS) {
//        printf("Could not find HeightTrajectory:offset!\n");
//        return false;
//      }
//      if (!loadHeightTrajectory(hTrajectory)) {
//        printf("problem\n");
//        return false;
//      }
//    }
//
//  }

  return true;
}

bool TorsoControlDynamicGaitPerceptedTerrain::loadHeightTrajectory(const TiXmlHandle &hTrajectory,  rbf::PeriodicRBF1DC1& trajectory)
{
  TiXmlElement* pElem;
  int iKnot;
  double t, value;
  std::vector<double> tValues, xValues;


  TiXmlElement* child = hTrajectory.FirstChild().ToElement();
   for( child; child; child=child->NextSiblingElement() ){
      if (child->QueryDoubleAttribute("t", &t)!=TIXML_SUCCESS) {
        printf("Could not find t of knot!\n");
        return false;
      }
      if (child->QueryDoubleAttribute("v", &value)!=TIXML_SUCCESS) {
        printf("Could not find v of knot!\n");
        return false;
      }
      tValues.push_back(t);
      xValues.push_back(value);
//      printf("t=%f, v=%f\n", t, value);
   }
   trajectory.setRBFData(tValues, xValues);


  return true;
}

} /* namespace loco */