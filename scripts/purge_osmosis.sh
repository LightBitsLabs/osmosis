#!/bin/bash

help=${help:-false}
labels_path=${labels_file:-/scripts/labels.list}
osmosis_bin_path=${osmosis_bin_path:-/osmosis.bin}
objectstore_path=${object_store_path:-/data}

while [ $# -gt 0 ]; do

   if [[ $1 == *"--"* ]]; then
        param="${1/--/}"
        declare $param="$2"
   fi

  shift
done

if [[ $help != "false" ]]; then
      cat Readme.md
      exit
fi

echo -e "\nGoing to use the following parameters:\n"
echo -e "   labels_path:      '$labels_path'"
echo -e "   osmosis_bin_path: '$osmosis_bin_path'"
echo -e "   objectstore_path: '$objectstore_path'"

cd /data/labels

echo -e "\nErasing labels..."
for label in $(cat $labels_path); do
    echo -e "   erasing label: $label"
    ls | grep "$label" | while read line; do /osmosis.bin eraselabel $line ; done
done
echo -e "Erase finished!\n"

echo -e "Going to purge osmosis, this may take a while..."
echo -e "   Executing command: $osmosis_bin_path purge --objectStoreRootPath=$objectstore_path\n"
$osmosis_bin_path purge --objectStoreRootPath=$objectstore_path
echo -e "\nAll done! Exiting...\n"
