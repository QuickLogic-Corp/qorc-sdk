all:	1micvr_app

.PHONY:qf_1micvr_app
1micvr_app:
	cd qf_$@/GCC_Project && make

CLEAN_TARGET += 1micvr_app
1micvr_app_clean:
	cd qf_$(subst _clean,,$@)/GCC_Project && make clean

$(info CLEAN_TARGET $(CLEAN_TARGET))
clean:	$(CLEAN_TARGET:%=%_clean)

