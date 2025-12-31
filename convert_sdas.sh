for filename in driver/*.inc; do
    node convert_wladx_sdas.js "driver/$(basename $filename)" "driver_sdas/$(basename $filename)"
done