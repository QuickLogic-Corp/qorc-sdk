all:	1micvr_app 2micvr_app vr_raw_app host_app

.PHONY:qf_1micvr_app
1micvr_app:
	cd qf_$@/GCC_Project && make

CLEAN_TARGET += 1micvr_app
1micvr_app_clean:
	cd qf_$(subst _clean,,$@)/GCC_Project && make clean

.PHONY:qf_2micvr_app
2micvr_app:
	cd qf_$@/GCC_Project && make

CLEAN_TARGET += 2micvr_app
2micvr_app_clean:
	cd qf_$(subst _clean,,$@)/GCC_Project && make clean

.PHONY:qf_vr_raw_app
vr_raw_app:
	cd qf_$@/GCC_Project && make

CLEAN_TARGET += vr_raw_app
vr_raw_app_clean:
	cd qf_$(subst _clean,,$@)/GCC_Project && make clean

.PHONY:qf_host_app
host_app:
	cd qf_$@/GCC_Project && make

CLEAN_TARGET += host_app
host_app_clean:
	cd qf_$(subst _clean,,$@)/GCC_Project && make clean

$(info CLEAN_TARGET $(CLEAN_TARGET))
clean:	$(CLEAN_TARGET:%=%_clean)

