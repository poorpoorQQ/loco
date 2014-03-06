/*!
* @file     GaitPatternAPS.hpp
* @author   Christian Gehring
* @date     Jun, 2013
* @version  1.0
* @ingroup
* @brief
*/

#include "loco/gait_pattern/GaitPatternAPS.hpp"


#include "tinyxml.h"

#include <cmath>
#include <vector>

namespace loco {

GaitPatternAPS::GaitPatternAPS():velocity_(0.0) {

	numGaitCycles = 0;

	for (int iLeg=0; iLeg<4; iLeg++)  {
		stancePhases_[iLeg] = 0.0;
		swingPhases_[iLeg] = -1.0;
	}

}

GaitPatternAPS::~GaitPatternAPS() {


}



double GaitPatternAPS::getSwingPhaseForLeg(int iLeg) {
	return GaitAPS::getSwingPhase(iLeg);

}


double GaitPatternAPS::getStanceDuration(int iLeg) {
	return GaitAPS::getStanceDuration(iLeg);
}

double GaitPatternAPS::getStrideDuration()
{
	return GaitAPS::getStrideDuration();
}

void GaitPatternAPS::setStrideDuration(double strideDuration)
{
	GaitAPS::setStrideDuration(strideDuration);
}


double GaitPatternAPS::getStancePhaseForLeg(int iLeg){
	return getStancePhase(iLeg);

}



bool GaitPatternAPS::loadParameters(const TiXmlHandle &hParameterSet)
{
	double value, liftOff, touchDown;
	int iLeg;
	TiXmlElement* pElem;

	/* gait pattern */
	pElem = hParameterSet.FirstChild("GaitPattern").Element();
	if (!pElem) {
		printf("Could not find GaitPattern\n");
		return false;
	}



	/* APS */


	pElem = hParameterSet.FirstChild("GaitPattern").FirstChild("APS").Element();
	if(!pElem) {
		printf("Could not find GaitPattern:APS\n");
		return false;
	}

	pElem = hParameterSet.FirstChild("GaitPattern").FirstChild("APS").FirstChild("GaitDiagram").Element();
	if(!pElem) {
		printf("Could not find GaitPattern:APS:GaitDiagram\n");
		return false;
	}
	if (pElem->QueryDoubleAttribute("cycleDuration", &initAPS_.foreCycleDuration_)!=TIXML_SUCCESS) {
		printf("Could not find GaitPattern:APS:GaitDiagram:cycleDuration\n");
		return false;
	}
	initAPS_.hindCycleDuration_ = initAPS_.foreCycleDuration_;


	if (pElem->QueryDoubleAttribute("foreLag", &initAPS_.foreLag_)!=TIXML_SUCCESS) {
		printf("Could not find GaitPattern:APS:GaitDiagram:foreLag\n");
		return false;
	}
	if (pElem->QueryDoubleAttribute("hindLag", &initAPS_.hindLag_)!=TIXML_SUCCESS) {
		printf("Could not find GaitPattern:APS:GaitDiagram:hindLag\n");
		return false;
	}
	if (pElem->QueryDoubleAttribute("pairLag", &initAPS_.pairLag_)!=TIXML_SUCCESS) {
		printf("Could not find GaitPattern:APS:GaitDiagram:pairLag\n");
		return false;
	}
	if (pElem->QueryDoubleAttribute("foreDutyFactor", &initAPS_.foreDutyFactor_)!=TIXML_SUCCESS) {
		printf("Could not find GaitPattern:APS:GaitDiagram:foreDutyFactor\n");
		return false;
	}
	if (pElem->QueryDoubleAttribute("hindDutyFactor", &initAPS_.hindDutyFactor_)!=TIXML_SUCCESS) {
		printf("Could not find GaitPattern:APS:GaitDiagram:hindDutyFactor\n");
		return false;
	}


	pElem = hParameterSet.FirstChild("GaitPattern").FirstChild("APS").FirstChild("CycleDuration").Element();
	if(pElem) {
		if (pElem->QueryDoubleAttribute("minVel", &initAPS_.foreCycleDurationMinVelocity_)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:CycleDuration:minVel\n");
			return false;
		}
		initAPS_.hindCycleDurationMinVelocity_ = initAPS_.foreCycleDurationMinVelocity_;

		if (pElem->QueryDoubleAttribute("minVelValue", &initAPS_.foreCycleDurationMinVelocityValue_)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:CycleDuration:minVelValue\n");
			return false;
		}
		initAPS_.hindCycleDurationMinVelocityValue_ = initAPS_.foreCycleDurationMinVelocityValue_;

		if (pElem->QueryDoubleAttribute("maxVel", &initAPS_.foreCycleDurationMaxVelocity_)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:CycleDuration:maxVel\n");
			return false;
		}
		initAPS_.hindCycleDurationMaxVelocity_ = initAPS_.foreCycleDurationMaxVelocity_;

		if (pElem->QueryDoubleAttribute("maxVelValue", &initAPS_.foreCycleDurationMaxVelocityValue_)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:CycleDuration:maxVelValue\n");
			return false;
		}
		initAPS_.hindCycleDurationMaxVelocityValue_ = initAPS_.foreCycleDurationMaxVelocityValue_;

		std::string law;
		if (pElem->QueryStringAttribute("law", &law)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:CycleDuration:law\n");
			return false;
		}
		if (law == "none") {
			initAPS_.foreCycleDurationLaw_ = APS::APS_VL_NONE;
			initAPS_.hindCycleDurationLaw_ = APS::APS_VL_NONE;
		} else  if(law == "lin") {
			initAPS_.foreCycleDurationLaw_ = APS::APS_VL_LIN;
			initAPS_.hindCycleDurationLaw_ = APS::APS_VL_LIN;
		} else  if(law == "log") {
			initAPS_.foreCycleDurationLaw_ = APS::APS_VL_LOG;
			initAPS_.hindCycleDurationLaw_ = APS::APS_VL_LOG;
		} else  if(law == "exp") {
			initAPS_.foreCycleDurationLaw_ = APS::APS_VL_EXP;
			initAPS_.hindCycleDurationLaw_ = APS::APS_VL_EXP;
		}
	}


	pElem = hParameterSet.FirstChild("GaitPattern").FirstChild("APS").FirstChild("ForeDutyFactor").Element();
	if(pElem) {
		if (pElem->QueryDoubleAttribute("minVel", &initAPS_.foreDutyFactorMinVelocity_)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:ForeDutyFactor:minVel\n");
			return false;
		}

		if (pElem->QueryDoubleAttribute("minVelValue", &initAPS_.foreDutyFactorMinVelocityValue_)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:ForeDutyFactor:minVelValue\n");
			return false;
		}

		if (pElem->QueryDoubleAttribute("maxVel", &initAPS_.foreDutyFactorMaxVelocity_)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:ForeDutyFactor:maxVel\n");
			return false;
		}

		if (pElem->QueryDoubleAttribute("maxVelValue", &initAPS_.foreDutyFactorMaxVelocityValue_)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:ForeDutyFactor:maxVelValue\n");
			return false;
		}

		std::string law;
		if (pElem->QueryStringAttribute("law", &law)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:ForeDutyFactor:law\n");
			return false;
		}
		if (law == "none") {
			initAPS_.foreDutyFactorLaw_ = APS::APS_VL_NONE;
		} else  if(law == "lin") {
			initAPS_.foreDutyFactorLaw_ = APS::APS_VL_LIN;
		} else  if(law == "log") {
			initAPS_.foreDutyFactorLaw_ = APS::APS_VL_LOG;
		} else  if(law == "exp") {
			initAPS_.foreDutyFactorLaw_ = APS::APS_VL_EXP;
		}
	}


	pElem = hParameterSet.FirstChild("GaitPattern").FirstChild("APS").FirstChild("HindDutyFactor").Element();
	if(pElem) {
		if (pElem->QueryDoubleAttribute("minVel", &initAPS_.hindDutyFactorMinVelocity_)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:HindDutyFactor:minVel\n");
			return false;
		}

		if (pElem->QueryDoubleAttribute("minVelValue", &initAPS_.hindDutyFactorMinVelocityValue_)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:HindDutyFactor:minVelValue\n");
			return false;
		}

		if (pElem->QueryDoubleAttribute("maxVel", &initAPS_.hindDutyFactorMaxVelocity_)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:HindDutyFactor:maxVel\n");
			return false;
		}

		if (pElem->QueryDoubleAttribute("maxVelValue", &initAPS_.hindDutyFactorMaxVelocityValue_)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:HindDutyFactor:maxVelValue\n");
			return false;
		}

		std::string law;
		if (pElem->QueryStringAttribute("law", &law)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:HindDutyFactor:law\n");
			return false;
		}
		if (law == "none") {
			initAPS_.hindDutyFactorLaw_ = APS::APS_VL_NONE;
		} else  if(law == "lin") {
			initAPS_.hindDutyFactorLaw_ = APS::APS_VL_LIN;
		} else  if(law == "log") {
			initAPS_.hindDutyFactorLaw_ = APS::APS_VL_LOG;
		} else  if(law == "exp") {
			initAPS_.hindDutyFactorLaw_ = APS::APS_VL_EXP;
		}
	}

	pElem = hParameterSet.FirstChild("GaitPattern").FirstChild("APS").FirstChild("PairLag").Element();
	if(pElem) {
		if (pElem->QueryDoubleAttribute("minVel", &initAPS_.pairLagMinVelocity_)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:PairLag:minVel\n");
			return false;
		}

		if (pElem->QueryDoubleAttribute("minVelValue", &initAPS_.pairLagMinVelocityValue_)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:PairLag:minVelValue\n");
			return false;
		}

		if (pElem->QueryDoubleAttribute("maxVel", &initAPS_.pairLagMaxVelocity_)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:PairLag:maxVel\n");
			return false;
		}

		if (pElem->QueryDoubleAttribute("maxVelValue", &initAPS_.pairLagMaxVelocityValue_)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:PairLag:maxVelValue\n");
			return false;
		}

		std::string law;
		if (pElem->QueryStringAttribute("law", &law)!=TIXML_SUCCESS) {
			printf("Could not find GaitPattern:APS:GaitDiagram:PairLag:law\n");
			return false;
		}
		if (law == "none") {
		  initAPS_.pairLagLaw_ = APS::APS_VL_NONE;
		} else  if(law == "lin") {
		  initAPS_.pairLagLaw_ = APS::APS_VL_LIN;
		} else  if(law == "log") {
		  initAPS_.pairLagLaw_ = APS::APS_VL_LOG;
		} else  if(law == "exp") {
		  initAPS_.pairLagLaw_ = APS::APS_VL_EXP;
		}
	}

	// save to remove?
//	GaitAPS::initAPS(initAPS_, dt);



	return true;
}

bool GaitPatternAPS::initialize(const APS& aps, double dt) {
  GaitAPS::initAPS(aps, dt);

  return true;
}

bool GaitPatternAPS::initialize(double dt) {
  GaitAPS::initAPS(initAPS_, dt);

  return true;
}


bool GaitPatternAPS::saveParameters(TiXmlHandle &hParameterSet)
{

	TiXmlElement* pElem;

	/* gait pattern */
	pElem = hParameterSet.FirstChild("GaitPattern").Element();
	if (!pElem) {
		printf("Could not find GaitPattern\n");
		return false;
	}



	return true;
}


unsigned long int GaitPatternAPS::getNGaitCycles()
{
	return GaitAPS::getNumGaitCycles();
}




void GaitPatternAPS::setVelocity(double value)
{
	velocity_ = value;
	for (int iLeg=0; iLeg<4; iLeg++) {
//		currentAPS[iLeg]->setParameters(value);
		nextAPS[iLeg]->setParameters(value);
		nextNextAPS[iLeg]->setParameters(value);
	}
}

double GaitPatternAPS::getVelocity()
{
	return velocity_;
}

void GaitPatternAPS::advance(double dt) {
  GaitAPS::advance(dt);
}

bool GaitPatternAPS::shouldBeLegGrounded(int iLeg) {
  GaitAPS::shouldBeGrounded(iLeg);
}

double GaitPatternAPS::getStridePhase() {
  GaitAPS::getStridePhase();
}



} // namespace loco
