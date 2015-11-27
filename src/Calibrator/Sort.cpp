#include "Sort.h"
#include <cmath>
#include "../Util.h"
#include "../File/File.h"
#include "../ParameterFile/ParameterFile.h"
#include "../Downscaler/Pressure.h"
#include "../TrainingData.h"
CalibratorSort::CalibratorSort(Variable::Type iVariable, const Options& iOptions) :
      Calibrator(iOptions),
      mVariable(iVariable) {
}
bool CalibratorSort::calibrateCore(File& iFile, const ParameterFile* iParameterFile) const {
   int nLat = iFile.getNumLat();
   int nLon = iFile.getNumLon();
   int nEns = iFile.getNumEns();
   int nTime = iFile.getNumTime();
   vec2 lats = iFile.getLats();
   vec2 lons = iFile.getLons();
   vec2 elevs = iFile.getElevs();

   // Loop over offsets
   for(int t = 0; t < nTime; t++) {
      Field& field = *iFile.getField(mVariable, t);

      #pragma omp parallel for
      for(int i = 0; i < nLat; i++) {
         for(int j = 0; j < nLon; j++) {
            std::vector<float> values = field(i,j);
            std::sort(values.begin(), values.end());

            // Create a new array with all the missing values
            // at the end
            std::vector<float> missingLast(nEns, Util::MV);
            int counter = 0;
            for(int e = 0; e < nEns; e++) {
               if(Util::isValid(values[e])) {
                  missingLast[counter] = values[e];
                  counter++;
               }
            }
            for(int e = 0; e < nEns; e++) {
               field(i,j,e) = missingLast[e];
            }
         }
      }
   }
   return true;
}

std::string CalibratorSort::description() {
   std::stringstream ss;
   ss << Util::formatDescription("-c sort", "Sorts the ensemble members from lowest to highest. Any missing members are placed last.") << std::endl;
   return ss.str();
}