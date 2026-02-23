In this decoding_script folder, this contains 2 different decoding script and each doing the same basic decoding logic as intended. The difference is one of them is configured to be run on Python and the other on C/C++ and decoding_script_check.cpp also features engineering the dataset so that it is ready for model evaluation

decoding_script_check.cpp (Instructions to run):
In order to run this, the most simple way would be to run using Windows C/C++ debugger, after running you would receive a file called iforest_features.csv, this can then directly be placed inside the iforest model. 

decoding.ipynb:
This file contains a simpler decoding script that only decodes the signals and doesn't do any feature engineering, to run it you must ensure you have Python and Scikit-Learn inside on Vs Code. 
Another easier way to run it would be to upload this code into Google Colab, however, you would also need to upload the can_log_dataset.csv file in this folder. 

