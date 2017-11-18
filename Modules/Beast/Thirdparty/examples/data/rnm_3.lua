function setup()
	bset("gather.sampletype", "indirect illumination")
	bset("gather.space", "tangent space")
	bset("gather.distribution", "equiareal")
	bset("gather.minsamples", 300)
	bset("gather.maxsamples", 600)
	bset("gather.coneangle", 180)
	bset("output.size", 9)
end


function bake()

	basis1 = vec3(0.8165, 0.0, 0.577)
	basis2 = vec3(-0.408, 0.707, 0.577)
	basis3 = vec3(-0.408, -0.707, 0.577)

	rgb1 = getSampleProjectedSum(basis1, false)
	rgb2 = getSampleProjectedSum(basis2, false)
	rgb3 = getSampleProjectedSum(basis3, false)

	n = getSampleCount()
	rgb1 = (rgb1 * 2.0) / n
	rgb2 = (rgb2 * 2.0) / n
	rgb3 = (rgb3 * 2.0) / n

	array = {rgb1[1], rgb1[2], rgb1[3],
		 rgb2[1], rgb2[2], rgb2[3],
		 rgb3[1], rgb3[2], rgb3[3]}

	return array

end
