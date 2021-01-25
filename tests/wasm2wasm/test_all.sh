

RED='\033[0;31m'
NC='\033[0m' # No Color

clean() {
	rm *.mirror.*
	rm *.wat.wasm
	rm *.vv.*
	rm *.ll
	rm *.bc
	echo "Clean!"
}

error(){
	printf "${RED}Fail in $1${NC}\n"
	cat $1
	clean
	exit 1
}


check_ops(){

	for t in $1
	do
		for op in $2
		do
			export INSTR="$t.$op"
			export T=$t
			export R=$t
			cp binops.TEMPLATE.wat binops.wat
			perl  -pe  's/OP/$ENV{INSTR}/g' -i binops.wat
			perl  -pe  's/T/$ENV{T}/g' -i binops.wat
			perl  -pe  's/R/$ENV{R}/g' -i binops.wat
			echo "Checking $INSTR"

			bash test1.sh binops.wat

			if ! grep -q $INSTR "binops.wat.mirror.wat"; then

				printf "${RED}$INSTR${NC}\n"
				error binops.wat.mirror.wat
			fi
		done
	done
}


TYPES_I="i32 i64"
BINOPS_I="add sub mul xor and or shl shr_s shr_u div_s div_u rem_s rem_u"


#TYPES_F="i32 i64"
#BINOPS_F="add sub mul"


check_ops "$TYPES_I" "$BINOPS_I"

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

bash test1.sh f32_floor.wat

if ! grep -q f32.floor "f32_floor.wat.mirror.wat"; then
	error f32_floor.wat.mirror.wat
fi


bash test1.sh f64_floor.wat

if ! grep -q f64.floor "f64_floor.wat.mirror.wat"; then
	error f64_floor.wat.mirror.wat
fi


clean