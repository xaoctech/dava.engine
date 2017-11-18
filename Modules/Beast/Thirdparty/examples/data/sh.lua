shbands = 2;
shbasiscount = shbands*shbands;


function setup()

	bset("gather.sampletype", "indirect illumination")
	bset("gather.space", "world space")
	bset("gather.distribution", "equiareal")
	bset("gather.minsamples", 700)
	bset("gather.maxsamples", 1000)
	bset("gather.coneangle", 360)
	bset("gather.cosweighdynamic", false)

	bset("basis.type", "sh")
	bset("basis.rgb", true)
	bset("basis.sh.bands", shbands)

	bset("output.size", 3 * shbasiscount)

end


function bake()
	red = getBasisCoefficients(1)
	grn = getBasisCoefficients(2)
	blu = getBasisCoefficients(3)

	result = {}
	for i = 1, shbasiscount do
		result[i] = red[i]
	end
	for i = 1, shbasiscount do
		result[i + shbasiscount] = grn[i]
	end
	for i = 1, shbasiscount do
		result[i + 2*shbasiscount] = blu[i]
	end

	return result
end
