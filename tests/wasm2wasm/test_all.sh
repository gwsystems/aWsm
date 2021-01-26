

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
	printf "${RED} ${NO_CHANGES_ICON} Fail in $1${NC}\n"
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
	done
}


TYPES_I="i32 i64"
BINOPS_I="add sub mul xor and or shl shr_s shr_u div_s div_u rem_s rem_u"

RELOPS_I="eq ne lt_s lt_u gt_s gt_u ge_s ge_u le_u le_s"
RELOPS_F="eq ne lt gt le ge"

TYPES_F="f32 f64"
BINOPS_F="add sub mul div copysign"

UNOPS_I="clz ctz popcnt"
UNOPS_F="abs neg sqrt ceil floor nearest trunc"

check_unops "$TYPES_I" "$UNOPS_I"
check_unops "$TYPES_F" "$UNOPS_F"

check_ops "$TYPES_I" "$BINOPS_I"
check_ops "$TYPES_F" "$BINOPS_F"

check_relops "$TYPES_I" "$RELOPS_I"
check_relops "$TYPES_F" "$RELOPS_F"

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


clean