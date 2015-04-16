all: lof/lof.l1 svm/svm-train frac/frac

lof/lof.l1:  
	$(MAKE) -C lof

frac/frac:
	$(MAKE) -C frac

svm/svm-train:
	$(MAKE) -C svm

clean:
	$(MAKE) clean -C lof
	$(MAKE) clean -C svm
	$(MAKE) clean -C frac

distclean:
	$(MAKE) distclean -C lof
	$(MAKE) distclean -C svm
	$(MAKE) distclean -C frac
	/bin/rm -rf ./tmp

gseaclean: 
	/bin/rm -rf `date +"%b%d" | tr '[:upper:]' '[:lower:]'` 
	/bin/rm -rf ~/gsea_home/reports_cache_foo

build: distclean all
