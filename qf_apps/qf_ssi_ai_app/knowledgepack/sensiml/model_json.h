#ifndef __MODEL_JSON_H__
#define __MODEL_JSON_H__

const char recognition_model_string_json[] = {" \
		{\"NumModels\": 1, \"ModelIndexes\": {\"0\": \"slide_rank_0\"}, \"ModelDescriptions\": [{\"Name\": \"slide_rank_0\", \"ClassMaps\": {\"1\": \"Horizontal\", \"2\": \"Stationary\", \"3\": \"Vertical\", \"0\": \"Unknown\"}, \"ModelType\": \"Decision Tree Ensemble\", \"FeatureNames\": [\"gen_0009_AccelerometerZStd\", \"gen_0016_AccelerometerXSum\", \"gen_0021_AccelerometerZmaximum\", \"gen_0024_AccelerometerZminimum\", \"gen_0037_AccelerometerXMean\", \"gen_0042_AccelerometerZ100Percentile\", \"gen_0045_AccelerometerXMaxP2PGlobalDC\", \"gen_0046_AccelerometerYMaxP2PGlobalDC\", \"gen_0050_AccelerometerZMaxP2P1stHalfAC\", \"gen_0053_AccelerometerZP2P\", \"gen_0056_AccelerometerZMaxP2PGlobalAC\", \"gen_0086_AccelerometerZMeanCrossingRate\"]}]} \
"
};

int recognition_model_string_json_len = sizeof(recognition_model_string_json);

#endif /* __MODEL_JSON_H__ */