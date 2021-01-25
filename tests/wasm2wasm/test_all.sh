

clean() {
	rm *.mirror.*
	rm *.wat.wasm
	rm *.vv.*
	rm *.ll
	rm *.bc
	echo "Clean!"
}

error(){
	RED='\033[0;31m'
	NC='\033[0m' # No Color
	printf "${RED}Fail in $1${NC}\n"
	cat $1
	clean
	exit 1
}

bash test1.sh binops.wat

if ! grep -q i32.add "binops.wat.mirror.wat"; then
	error binops.wat.mirror.wat
fi

bash test1.sh binops2.wat

if ! grep -q i32.sub "binops2.wat.mirror.wat"; then
	error binops2.wat.mirror.wat
fi

bash test1.sh binops3.wat

if ! grep -q i32.shl "binops3.wat.mirror.wat"; then
	error binops3.wat.mirror.wat
fi

bash test1.sh f32_min.wat

if ! grep -q f32.min "f32_min.wat.mirror.wat"; then
	error f32_min.wat.mirror.wat
fi

bash test1.sh f32_max.wat

if ! grep -q f32.max "f32_max.wat.mirror.wat"; then
	error f32_max.wat.mirror.wat
fi


bash test1.sh f64_max.wat

if ! grep -q f64.max "f64_max.wat.mirror.wat"; then
	error f64_max.wat.mirror.wat
fi


bash test1.sh f64_min.wat

if ! grep -q f64.min "f64_min.wat.mirror.wat"; then
	error f64_min.wat.mirror.wat
fi


bash test1.sh rot.wat --expand-rot true 

if ! grep -q "i32.rotl" rot.wat.mirror.wat ; then
	error rot.wat.mirror.wat
fi

bash test1.sh rot3.wat --expand-rot true 

if ! grep -q "i64.rotl" rot3.wat.mirror.wat ; then
	error rot3.wat.mirror.wat
fi


bash test1.sh rot2.wat --expand-rot true 
if ! grep -q "i32.rotr" rot2.wat.mirror.wat ; then
	error rot2.wat.mirror.wat
fi


bash test1.sh rot4.wat --expand-rot true 
if ! grep -q "i64.rotr" rot4.wat.mirror.wat ; then
	error rot4.wat.mirror.wat
fi


bash test1.sh rot2.wat --expand-rot false
cat  rot2.wat.mirror.wat

bash test1.sh f32_floor.wat

if ! grep -q f32.floor "f32_floor.wat.mirror.wat"; then
	error f32_floor.wat.mirror.wat
fi

clean