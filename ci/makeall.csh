echo "Entering qf_apps from"
pwd
(cd qf_apps && make all)
echo "Entering qf_testapps from"
pwd
(cd qf_testapps && make all)
pwd
(cd qf_vr_apps && make all)
