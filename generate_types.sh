#!/bin/bash

# DDS Module type generation script

echo "Generating types for DDS Module ussing FastDDS-Gen"

red='\E[1;31m'
yellow='\E[1;33m'
textreset='\E[1;0m'

current_dir=$(git rev-parse --show-toplevel)

if [[ ! "$(pwd -P)" -ef "$current_dir" ]]; then
    echo -e "${red}This script must be executed in the repository root directory.${textreset}"
    exit -1
fi

if [[ -z "$(which fastddsgen)" ]]; then
    echo "Cannot find fastddsgen. Please, include it in PATH environment variable"
    exit -1
fi

echo "Processing DynamicTypesCollection.idl ..."

cd ddsenabler_participants/include/ddsenabler_participants/types/dynamic_types_collection
fastddsgen -replace DynamicTypesCollection.idl

mv DynamicTypesCollectionCdrAux.ipp ../../../../src/cpp/types/dynamic_types_collection/
mv DynamicTypesCollectionPubSubTypes.cxx ../../../../src/cpp/types/dynamic_types_collection/
mv DynamicTypesCollectionTypeObjectSupport.cxx ../../../../src/cpp/types/dynamic_types_collection/

sed -i 's+"DynamicTypesCollectionCdrAux.hpp"+ <ddsenabler_participants/types/dynamic_types_collection/DynamicTypesCollectionCdrAux.hpp>+'  ../../../../src/cpp/types/dynamic_types_collection/DynamicTypesCollectionCdrAux.ipp

sed -i 's+"DynamicTypesCollectionPubSubTypes.hpp"+ <ddsenabler_participants/types/dynamic_types_collection/DynamicTypesCollectionPubSubTypes.hpp>+'  ../../../../src/cpp/types/dynamic_types_collection/DynamicTypesCollectionPubSubTypes.cxx
sed -i 's+"DynamicTypesCollectionCdrAux.hpp"+ <ddsenabler_participants/types/dynamic_types_collection/DynamicTypesCollectionCdrAux.hpp>+'  ../../../../src/cpp/types/dynamic_types_collection/DynamicTypesCollectionPubSubTypes.cxx
sed -i 's+"DynamicTypesCollectionTypeObjectSupport.hpp"+ <ddsenabler_participants/types/dynamic_types_collection/DynamicTypesCollectionTypeObjectSupport.hpp>+'  ../../../../src/cpp/types/dynamic_types_collection/DynamicTypesCollectionPubSubTypes.cxx

sed -i 's+"DynamicTypesCollectionTypeObjectSupport.hpp"+ <ddsenabler_participants/types/dynamic_types_collection/DynamicTypesCollectionTypeObjectSupport.hpp>+'  ../../../../src/cpp/types/dynamic_types_collection/DynamicTypesCollectionTypeObjectSupport.cxx
sed -i 's+"DynamicTypesCollection.hpp"+ <ddsenabler_participants/types/dynamic_types_collection/DynamicTypesCollection.hpp>+'  ../../../../src/cpp/types/dynamic_types_collection/DynamicTypesCollectionTypeObjectSupport.cxx
 
echo "DynamicTypesCollection.idl generation completed successfully."