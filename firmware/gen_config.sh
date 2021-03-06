#!/bin/bash

echo "This script reads rusefi_config.txt and produces firmware persistent configuration headers"
echo "The storage section of rusefi.ini is updated as well"

rm -f gen_config.log
rm -f gen_config_board.log

bash gen_config_default.sh
[ $? -eq 0 ] || { echo "ERROR generating default"; exit 1; }

for BOARD in "microrusefi mre_f7" "microrusefi mre_f4" "frankenso frankenso_na6" "prometheus prometheus_469" "prometheus prometheus_405" "proteus proteus_f7" "proteus proteus_f4"; do
 BOARD_NAME="${BOARD% *}"
 BOARD_SHORT_NAME="${BOARD#* }"
 bash gen_config_board.sh $BOARD_NAME $BOARD_SHORT_NAME
 [ $? -eq 0 ] || { echo "ERROR generating board $BOARD_NAME $BOARD_SHORT_NAME"; exit 1; }
done

cd config/boards/kinetis/config
bash gen_config.sh
[ $? -eq 0 ] || { echo "ERROR generating board kinetis kin"; exit 1; }

exit 0
