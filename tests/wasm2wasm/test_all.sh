

RED='\033[0;31m'
NC='\033[0m' # No Color
COLOR_GREEN="\e[1;32m"

CHANGES_ICON='\xE2\x9C\x94'
NO_CHANGES_ICON='\xE2\x9C\x94'

clean() {
	rm *.mirror.*
	rm *.wat.wasm
	rm *.vv.*
	rm *.ll
	rm *.bc
	echo "Clean!"
}

error(){
	printf "${RED} ${CHANGES_ICON} Fail in $1${NC}\n"
	cat $1
	cat error.txt
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

			echo -n "Checking $INSTR "

			bash test1.sh binops.wat

			if ! grep -q $INSTR "binops.wat.mirror.wat"; then

				printf "${RED}$INSTR${NC}\n"
				error binops.wat.mirror.wat
			fi

			printf " ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"
		done
		echo
	done
}


check_unops(){

	for t in $1
	do
		for op in $2
		do
			export INSTR="$t.$op"
			export T=$t
			export R=$t
			cp unops.TEMPLATE.wat unops.wat
			perl  -pe  's/OP/$ENV{INSTR}/g' -i unops.wat
			perl  -pe  's/T/$ENV{T}/g' -i unops.wat
			perl  -pe  's/R/$ENV{R}/g' -i unops.wat

			echo -n "Checking $INSTR "

			bash test1.sh unops.wat

			if ! grep -q $INSTR "unops.wat.mirror.wat"; then

				printf "${RED}$INSTR${NC}\n"
				error unops.wat.mirror.wat
			fi

			printf " ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"
		done
		echo
	done
}

check_relops(){

	for t in $1
	do
		for op in $2
		do
			export INSTR="$t.$op"
			export T=$t
			export R=$t
			cp relops.TEMPLATE.wat relops.wat
			perl  -pe  's/OP/$ENV{INSTR}/g' -i relops.wat
			perl  -pe  's/T/$ENV{T}/g' -i relops.wat
			perl  -pe  's/R/$ENV{R}/g' -i relops.wat

			echo -n "Checking $INSTR "

			bash test1.sh relops.wat

			if ! grep -q $INSTR "relops.wat.mirror.wat"; then

				printf "${RED}$INSTR${NC}\n"
				error relops.wat.mirror.wat
			fi

			printf " ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"
		done
		echo
	done
}

check_testops(){
	for t in $1
	do
		for op in $2
		do
			export INSTR="$t.$op"
			export T=$t
			export R=$t

			cp tops.TEMPLATE.wat tops.wat
			perl  -pe  's/OP/$ENV{INSTR}/g' -i tops.wat
			perl  -pe  's/T/$ENV{T}/g' -i tops.wat
			perl  -pe  's/R/$ENV{R}/g' -i tops.wat

			echo -n "Checking $INSTR "

			bash test1.sh tops.wat

			if ! grep -q $INSTR "tops.wat.mirror.wat"; then

				printf "${RED}$INSTR${NC}\n"
				error tops.wat.mirror.wat
			fi

			printf " ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"
		done
	done
}


check_extend(){
	for t in $1
	do
		for op in $2
		do
			export INSTR="$t.$op"
			export T=$t
			export R=$t

			cp extend.TEMPLATE.wat extend.wat
			perl  -pe  's/OP/$ENV{INSTR}/g' -i extend.wat
			perl  -pe  's/T/$ENV{T}/g' -i extend.wat
			perl  -pe  's/R/$ENV{R}/g' -i extend.wat

			echo -n "Checking $INSTR "

			bash test1.sh extend.wat

			if ! grep -q $INSTR "extend.wat.mirror.wat"; then

				printf "${RED}$INSTR${NC}\n"
				error extend.wat.mirror.wat
			fi

			printf " ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"
		done
		echo
	done
}

check_mem_store(){
	for t in $1
	do
		for op in $2
		do
			export INSTR="$t.$op"
			export T=$t

			cp mem_store.TEMPLATE.wat mem_store.wat
			perl  -pe  's/T/$ENV{T}/g' -i mem_store.wat
			perl  -pe  's/OP/$ENV{INSTR}/g' -i mem_store.wat

			echo -n "Checking $INSTR "

			bash test1.sh mem_store.wat

			if ! grep -q $INSTR "mem_store.wat.mirror.wat"; then

				printf "${RED}$INSTR${NC}\n"
				error mem_store.wat.mirror.wat
			fi

			printf " ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"
		done
		echo
	done
}



MEM_STORE_I32="store8 store16 store"

check_mem_store "i32" "$MEM_STORE_I32"
check_mem_store "i64" "store32 store"
check_mem_store "f32 f64" "store"


check_mem_load(){
	for t in $1
	do
		for op in $2
		do
			export INSTR="$t.$op"
			export T=$t
			export R=$t

			cp mem_load.TEMPLATE.wat mem_load.wat
			perl  -pe  's/OP/$ENV{INSTR}/g' -i mem_load.wat
			perl  -pe  's/T/$ENV{T}/g' -i mem_load.wat
			perl  -pe  's/R/$ENV{R}/g' -i mem_load.wat

			echo -n "Checking $INSTR "

			bash test1.sh mem_load.wat

			if ! grep -q $INSTR "mem_load.wat.mirror.wat"; then

				printf "${RED}$INSTR${NC}\n"
				error mem_load.wat.mirror.wat
			fi

			printf " ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"
		done
		echo
	done
}


check_mem_load "i32" "load8_u load8_s load16_s load16_u load"
check_mem_load "i64" "load32_s load32_u load"
check_mem_load "f32 f64" "load"

exit 1

BINOPS_I="add sub mul xor and or shl shr_s shr_u div_s div_u rem_s rem_u"

RELOPS_I=" eq ne lt_s lt_u gt_s gt_u ge_s ge_u le_u le_s"
RELOPS_F="eq ne lt gt le ge"

EXTEND_OPS_I64="extend32_s extend32_u"
EXTEND_OPS_I32="extend8_s extend16_s"

TYPES_F="f32 f64"
BINOPS_F="add sub mul div copysign"

UNOPS_I="clz ctz popcnt"
UNOPS_F="abs neg sqrt ceil floor nearest trunc"

T_OPS_I="eqz"

check_unops "$TYPES_I" "$UNOPS_I"
echo
check_unops "$TYPES_F" "$UNOPS_F"
echo
check_ops "$TYPES_I" "$BINOPS_I"
echo
check_ops "$TYPES_F" "$BINOPS_F"
echo
check_relops "$TYPES_I" "$RELOPS_I"
echo
check_relops "$TYPES_F" "$RELOPS_F"
echo
check_testops "$TYPES_I" "$T_OPS_I"

# TODO
#echo
#check_extend "i64" "$EXTEND_OPS_I64"
#echo
#check_extend "i32" "$EXTEND_OPS_I32"

bash test1.sh f32_min.wat

if ! grep -q f32.min "f32_min.wat.mirror.wat"; then
	error f32_min.wat.mirror.wat
fi

printf "Checking f32.min ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"

bash test1.sh f32_max.wat

if ! grep -q f32.max "f32_max.wat.mirror.wat"; then
	error f32_max.wat.mirror.wat
fi

printf "Checking f32.max ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"

bash test1.sh f64_max.wat

if ! grep -q f64.max "f64_max.wat.mirror.wat"; then
	error f64_max.wat.mirror.wat
fi

printf "Checking f64.max ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"

bash test1.sh f64_min.wat

if ! grep -q f64.min "f64_min.wat.mirror.wat"; then
	error f64_min.wat.mirror.wat
fi

printf "Checking f64.min ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"

bash test1.sh rot.wat --expand-rot true 

if ! grep -q "i32.rotl" rot.wat.mirror.wat ; then
	error rot.wat.mirror.wat
fi

printf "Checking i32.rotl ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"

bash test1.sh rot3.wat --expand-rot true 

if ! grep -q "i64.rotl" rot3.wat.mirror.wat ; then
	error rot3.wat.mirror.wat
fi

printf "Checking i64.rotl ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"

bash test1.sh rot2.wat --expand-rot true 
if ! grep -q "i32.rotr" rot2.wat.mirror.wat ; then
	error rot2.wat.mirror.wat
fi


printf "Checking i32.rotr ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"

bash test1.sh rot4.wat --expand-rot true 
if ! grep -q "i64.rotr" rot4.wat.mirror.wat ; then
	error rot4.wat.mirror.wat
fi

printf "Checking i64.rotr ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"

bash test1.sh f32_floor.wat

if ! grep -q f32.floor "f32_floor.wat.mirror.wat"; then
	error f32_floor.wat.mirror.wat
fi


printf "Checking f32.floor ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"

bash test1.sh f64_floor.wat

if ! grep -q f64.floor "f64_floor.wat.mirror.wat"; then
	error f64_floor.wat.mirror.wat
fi


printf "Checking f64.floor ${COLOR_GREEN} ${CHANGES_ICON} ${NC}\n"



bash test1.sh babbage.wat
cat babbage.wat.mirror.wat

clean