var reflinkdata = new Array();
var bIsRefLinkDivShown=false;

function openRefLinkDiv(e, symbolName)
{
	var evt = e ? e:window.event;

	if (typeof evt.stopPropagation != "undefined") {
	evt.stopPropagation();
    } else {
        evt.cancelBubble = true;
    }

	if (!getResults(symbolName))
		return;
	resizeRefLinkDiv(evt);
	bIsRefLinkDivShown=true;
	toggleRefLinkDiv();
}

function getResults(symbolName)
{
    o_linkdiv = document.getElementById("reflinkdiv");
    if (o_linkdiv == null || reflinkdata.length < 1)
	{ return false; }

	var a_reflinkmatches = new Array();
	for (var i_data = 0; i_data < reflinkdata.length; i_data++)
	{
		if (reflinkdata[i_data][0] == symbolName)
		{ a_reflinkmatches.push(reflinkdata[i_data]); }
	}

	if (a_reflinkmatches.length == 0)
	{ return false; }
	if (a_reflinkmatches.length == 1)
	{
		if (location.protocol == "mk:")
		{ window.location.href = a_reflinkmatches[0][1]; }
		else
		{ window.open (a_reflinkmatches[0][1], '_self', false); }
		return false;
	}

	o_linkdiv.innerHTML = "";
	for (var i_match = 0; i_match < a_reflinkmatches.length; i_match++)
	{ o_linkdiv.innerHTML += "<a href='" + a_reflinkmatches[i_match][1] + "' id='reflink_" + i_match + "'>" + a_reflinkmatches[i_match][2] + "</a><br />"; }
	return true;
}

function toggleRefLinkDiv() {
	oRefLinkDiv = document.getElementById("reflinkdiv");
    if (oRefLinkDiv == null)
	{ return; }
	if (bIsRefLinkDivShown) {
			oRefLinkDiv.style.visibility = "visible";
			oRefLinkDiv.style.display = "block";
		} else {
			oRefLinkDiv.style.visibility = "hidden";
			oRefLinkDiv.style.display = "none";
			}
}

function resizeRefLinkDiv (event) {
	oRefLinkDiv = document.getElementById("reflinkdiv");
    if (oRefLinkDiv == null)
	{ return; }
	oRefLinkDiv.style.display = "block"; //< for Chrome.

	var iDivWidth = oRefLinkDiv.offsetWidth + 10;
	var iDivHeight = oRefLinkDiv.offsetHeight + 10;
	var scrollTop;
	if(typeof pageYOffset!= 'undefined'){ scrollTop = pageYOffset; }
    else{ var B = document.body; var D = document.documentElement; D = (D.clientHeight)? D: B; scrollTop = D.scrollTop; }
	var scrollLeft;
	if(typeof pageXOffset!= 'undefined'){ scrollLeft = pageXOffset; }
    else{ var B = document.body; var D = document.documentElement; D = (D.clientWidth)? D: B; scrollLeft = D.scrollLeft; }

	var clientHeight = document.documentElement ? document.documentElement.clientHeight :
                                           document.body.clientHeight;
	var clientWidth = document.documentElement ? document.documentElement.clientWidth :
                                           document.body.clientWidth;

	var iLeftClickPos = event.clientX + scrollLeft;
	var iLeftPos = iLeftClickPos + 10;
	var iTopClickPos = event.clientY + scrollTop;
	var iTopPos = iTopClickPos + 10;
	var iRightPos = iLeftPos + iDivWidth;
	var iBottomPos = iTopPos + iDivHeight;
	var windowBottom = clientHeight + scrollTop;
	var windowRight = clientWidth + scrollLeft;
	if (iRightPos > windowRight) {
		iLeftPos = windowRight - iDivWidth;
	}
	if (iBottomPos > windowBottom) {
		iTopPos = windowBottom - iDivHeight;
	}

	oRefLinkDiv.style.left = iLeftPos + "px";
	oRefLinkDiv.style.top = iTopPos + "px";
}

function closeRefLinkDiv(event) {
	bIsRefLinkDivShown=false;
	toggleRefLinkDiv();
}
