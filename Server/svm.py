import pandas as pd
from sklearn import svm, preprocessing
from sklearn.ensemble import RandomForestClassifier, AdaBoostClassifier, BaggingClassifier
from sklearn.linear_model import LogisticRegression
from sklearn.linear_model.base import LinearClassifierMixin
from sklearn.neighbors import KNeighborsClassifier
from sklearn.neural_network import MLPClassifier
from sklearn.tree import DecisionTreeClassifier

svc = None
min_max_scaler = None

def train_model(): # id,height,weight,steps
	global svc,min_max_scaler
	train = pd.DataFrame(pd.read_csv("./data/train_data.csv")).sample(frac=1).as_matrix()
	x_train = pd.DataFrame(train[:, 1:])
	y_train = train[:, 0]
	feature_range = (0,1)
	min_max_scaler = preprocessing.MinMaxScaler(feature_range=feature_range)
	# x_train = min_max_scaler.fit_transform(x_train)
	print("Training...")
	# print(x_train)
	svc = svm.SVC().fit(x_train, y_train)
	# svc = RandomForestClassifier().fit(x_train, y_train)
	# svc = KNeighborsClassifier(n_neighbors=3).fit(x_train,y_train)
	# svc = DecisionTreeClassifier().fit(x_train,y_train)
	# svc = LogisticRegression().fit(x_train,y_train)
	# svc = MLPClassifier(hidden_layer_sizes=(100,),max_iter=2000,activation='relu',learning_rate='adaptive').fit(x_train,y_train)
	# svc = AdaBoostClassifier().fit(x_train,y_train)
	# svc = BaggingClassifier().fit(x_train,y_train)
	# svc = svm.LinearSVC().fit(x_train,y_train)
	print("Training Done...")

def predict(record):# record = [height,weight,steps]
	global svc,min_max_scaler
	if svc == None:
		train_model()
	# test_data = pd.DataFrame(pd.read_csv("./testing.csv")).fillna(0)
	# x_test = test_data.iloc[:,1:]
	# y_test = test_data.iloc[:, 0]
	x_test = []
	x_test.append(record)
	# y_test = ["person_name"]
	# x_test = min_max_scaler.fit_transform(x_test)
	print(x_test)
	predicted_result = svc.predict(x_test)
	# print("Original: ", y_test.as_matrix())
	print("Predicted:", predicted_result)
	# print("Original Predicted")
	# i=0
	# for result in predicted_result:
	    # print(y_test[i]," ",result)
	    # i+=1
	return predicted_result[0]
