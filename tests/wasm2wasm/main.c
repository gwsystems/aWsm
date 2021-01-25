int f(int a, int b){

	return ((a << (b & 31)) | (a >> ((-b) & 31)));
}

int g(int a, int b){

	return ((a >> (b & 31)) | (a << ((-b) & 31)));
}


float h(float a){

	return floor(a);
}