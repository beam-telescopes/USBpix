if [[ "x${DAQ_BASE}" -eq "x$" ]] || [[ ! -f "${DAQ_BASE}/Makefile" ]]
then
	echo "Please run 'source setup.sh' first."
	exit
fi

output_folder="${DAQ_BASE}/scan-build-result"

(set -x
	cd "${DAQ_BASE}" || exit $?
)

cd "${DAQ_BASE}" || exit $?

(set -x
	make clean || exit $?
)

(set -x
	make distclean || exit $?
)

(set -x
	scan-build bash setup.sh -restore || exit $?
)

mkdir -p "${output_folder}"

(set -x
	scan-build -analyze-headers -k -o "$output_folder" make $* || exit $?
)
