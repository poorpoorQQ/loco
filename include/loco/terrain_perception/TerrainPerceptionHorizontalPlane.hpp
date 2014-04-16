/*
 * TerrainPerceptionHorizontalPlane.hpp
 *
 *  Created on: Apr 15, 2014
 *      Author: gech
 */

#ifndef LOCO_TERRAINPERCEPTIONHORIZONTALPLANE_HPP_
#define LOCO_TERRAINPERCEPTIONHORIZONTALPLANE_HPP_

#include "loco/terrain_perception/TerrainPerceptionBase.hpp"
#include "loco/common/TerrainModelHorizontalPlane.hpp"
#include "loco/common/LegGroup.hpp"

namespace loco {

class TerrainPerceptionHorizontalPlane: public TerrainPerceptionBase {
 public:
  TerrainPerceptionHorizontalPlane(TerrainModelHorizontalPlane* terrainModel, LegGroup* legs);
  virtual ~TerrainPerceptionHorizontalPlane();

  virtual bool initialize(double dt);

  /*! Advance in time
   * @param dt  time step [s]
   */
  virtual bool advance(double dt);
 protected:
  TerrainModelHorizontalPlane* terrainModel_;
  LegGroup* legs_;
};

} /* namespace loco */

#endif /* LOCO_TERRAINPERCEPTIONHORIZONTALPLANE_HPP_ */