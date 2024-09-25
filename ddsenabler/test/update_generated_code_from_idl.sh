#!/usr/bin/env bash

set -e

files_to_exclude=(
    './resources/dds-types-test/IDL/relative_path_include.idl' # Relative path not working in current location.
)

files_not_needing_typeobject=(
    './resources/dds-types-test/IDL/declarations.idl'
    './resources/dds-types-test/IDL/external.idl'
)

files_needing_case_sensitive=(
)

files_needing_output_dir=(
    './resources/dds-types-test/IDL/aliases.idl|../../../resources/types'
    './resources/dds-types-test/IDL/annotations.idl|../../../resources/types'
    './resources/dds-types-test/IDL/appendable.idl|../../../resources/types'
    './resources/dds-types-test/IDL/arrays.idl|../../../resources/types'
    './resources/dds-types-test/IDL/bitsets.idl|../../../resources/types'
    './resources/dds-types-test/IDL/constants.idl|../../../resources/types'
    # './resources/dds-types-test/IDL/declarations.idl|../../../resources/types'
    './resources/dds-types-test/IDL/enumerations.idl|../../../resources/types'
    # './resources/dds-types-test/IDL/external.idl|../../../resources/types'
    './resources/dds-types-test/IDL/final.idl|../../../resources/types'
    './resources/dds-types-test/IDL/helpers/basic_inner_types.idl|../../../../resources/types/helpers'
    './resources/dds-types-test/IDL/inheritance.idl|../../../resources/types'
    './resources/dds-types-test/IDL/key.idl|../../../resources/types'
    './resources/dds-types-test/IDL/maps.idl|../../../resources/types'
    './resources/dds-types-test/IDL/member_id.idl|../../../resources/types'
    './resources/dds-types-test/IDL/mutable.idl|../../../resources/types'
    './resources/dds-types-test/IDL/optional.idl|../../../resources/types'
    './resources/dds-types-test/IDL/primitives.idl|../../../resources/types'
    # './resources/dds-types-test/IDL/relative_path_include.idl|../../../resources/types'
    './resources/dds-types-test/IDL/sequences.idl|../../../resources/types'
    './resources/dds-types-test/IDL/strings.idl|../../../resources/types'
    './resources/dds-types-test/IDL/structures.idl|../../../resources/types'
    './resources/dds-types-test/IDL/unions.idl|../../../resources/types'
)

files_needing_no_typesupport=(
)

red='\E[1;31m'
yellow='\E[1;33m'
textreset='\E[1;0m'

correct_dir=$(git rev-parse --show-toplevel)'/ddsenabler/test'

if [[ ! "$(pwd -P)" -ef "$correct_dir" ]]; then
    echo -e "${red}This script must be executed in the test directory.${textreset}"
    echo -e "${red}Correct dirrectory: $correct_dir"
    exit -1
fi

if [[ -z "$(which fastddsgen)" ]]; then
    echo "Cannot find fastddsgen. Please, include it in PATH environment variable"
    exit -1
fi

readarray -d '' idl_files < <(find . -type f -iname \*.idl -print0)

for del in ${files_to_exclude[@]}; do
    idl_files=("${idl_files[@]/$del/}")
done

idl_files=(${idl_files[@]/$files_to_exclude/})

ret_value=0

for idl_file in "${idl_files[@]}"; do
    idl_dir=$(dirname "$idl_file")
    file_from_gen=$(basename "$idl_file")

    echo -e "Processing ${yellow}$idl_file${textreset}"

    cd "${idl_dir}"

    # Detect if needs type_object.
    [[ ${files_not_needing_typeobject[*]} =~ $idl_file ]] && to_arg='-no-typeobjectsupport' || to_arg=''

    # Detect if needs case sensitive.
    [[ ${files_needing_case_sensitive[*]} =~ $idl_file ]] && cs_arg='-cs' || cs_arg=''

    [[ ${files_needing_no_typesupport[*]} =~ $idl_file ]] && nosupport_arg='-no-typesupport' || nosupport_arg=''

    # Detect if needs output directories.
    not_processed=true
    for od_entry in ${files_needing_output_dir[@]}; do
        if [[ $od_entry = $idl_file\|* ]]; then
            not_processed=false
            od_entry_split=(${od_entry//\|/ })
            for od_entry_split_element in ${od_entry_split[@]:1}; do
                od_arg="-d ${od_entry_split_element}"
                fastddsgen -replace -genapi $to_arg $cs_arg $od_arg "$file_from_gen" -no-dependencies
            done
            break
        fi
    done

    if $not_processed; then
        fastddsgen -replace -genapi $to_arg $cs_arg $nosupport_arg "$file_from_gen" -no-dependencies
    fi

    if [[ $? != 0 ]]; then
        ret_value=-1
    fi

    cd -
done

exit $ret_value
