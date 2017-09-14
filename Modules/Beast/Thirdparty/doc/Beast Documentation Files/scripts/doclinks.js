
var thisDocLanguage = "en-US";
var viewOnlineLabel = "View online";
var viewOnlineUrl = "https://gameware.autodesk.com/beast/developer/doc/2015/documentation/en/";
var downloadLabel = "Download";

function constructLink(href, label)
{
return '<a href="' + href + '" style="font-size:14px; color:#959595; text-decoration:none; padding-left: 15px;" target="_blank">' + label + '</a>';
}

function PrintDocOnlineLinks()
{
	var LinkToOnlineContents = '';
	for (var i_onlineversion = 0; i_onlineversion < DocOnlineVersions.length; i_onlineversion++)
	{
		var printLink = true;
		if (location.protocol != "file:" && DocOnlineVersions[i_onlineversion].lang == thisDocLanguage)
		{
			printLink = false;
		}
		if (printLink)
		{
			LinkToOnlineContents += constructLink(DocOnlineVersions[i_onlineversion].href, DocOnlineVersions[i_onlineversion].label);
		}
	}
	if (LinkToOnlineContents != "") { LinkToOnlineContents = '<span style="padding-left:20px;">' + viewOnlineLabel + '</span>' + LinkToOnlineContents; }
	else if (location.protocol == "file:") { LinkToOnlineContents = constructLink(viewOnlineUrl, viewOnlineLabel); } 
	return LinkToOnlineContents;
}

function PrintDocDownloadLinks()
{
	var LinkToDownloadContents = '';
	var ThisLanguageDownloads = DocDownloads[thisDocLanguage];
	if (ThisLanguageDownloads == undefined || ThisLanguageDownloads == null) { return ''; }
	for (var i_download = 0; i_download < ThisLanguageDownloads.length; i_download++)
	{
		var printLink = true;
		if (location.protocol == "file:" && ThisLanguageDownloads[i_download].label == "HTML")
		{
			printLink = false;
		}
		if (printLink)
		{
			LinkToDownloadContents += constructLink(ThisLanguageDownloads[i_download].href, ThisLanguageDownloads[i_download].label);
		}
	}
	if (LinkToDownloadContents != "") { LinkToDownloadContents = '<span style="padding-left:20px;">' + downloadLabel + '</span>' + LinkToDownloadContents; }

	return LinkToDownloadContents;
}

function PrintDocLinks()
{
	var o_downloadlinkdiv = document.getElementById("DocLinks");
	o_downloadlinkdiv.innerHTML =  PrintDocOnlineLinks() + PrintDocDownloadLinks();
	o_downloadlinkdiv.style.cssText = "align:right; padding-right:20px; padding-top:3px; font-size:14px; color:#b2b2b5; text-decoration:none";
}