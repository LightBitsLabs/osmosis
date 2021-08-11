
purge_osmosis.sh

The script erases all redundant labels from osmosis and then purge it

Optional arguments:

    --labels_path <arg>       ( default=/scripts/labels.list )
        file content should be labels seperated by newlines

    --osmosis_bin_path <arg>  ( default=/osmosis.bin )
        osmosis executable path

    --objectstore_path <arg>  ( default=/data )
        path where osmosis will keep its db

Examples:

    ./purge_osmosis.sh --labels_path /labels_to_remove.list
    ./purge_osmosis.sh --osmosis_bin_path /new_osmosis.bin
    ./purge_osmosis.sh --objectstore_path /temp_data


