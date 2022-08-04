#include <JuceHeader.h>

void convertHW5HW4CODM(XmlDocument& odf, File destination)
{
	//XmlDocument odf(source);
	auto Hauptwerk = odf.getDocumentElement();

	//TODO: Check if auto-compacted. If true, refuse to process and report error
	Hauptwerk->setAttribute("FileFormatVersion", "4.00");

	auto _General = Hauptwerk->getChildByAttribute("ObjectType", "_General");
	_General = _General->getChildByName("_General");

	auto Sys_ObjectID = _General->getChildByName("Sys_ObjectID");
	if (!Sys_ObjectID)
	{
		Sys_ObjectID = new XmlElement("Sys_ObjectID");
		Sys_ObjectID->addChildElement(_General->createTextElement("1"));
		_General->addChildElement(Sys_ObjectID);
	}

	Hauptwerk->writeTo(destination);
}

void convertHWX_to_HW4NATIVE(XmlDocument& odf, File destination)
{
	auto Hauptwerk = odf.getDocumentElement();

	//TODO: Check if auto-compacted. If true, refuse to process and report error

	//set sample set format to HW4
	Hauptwerk->setAttribute("FileFormatVersion", "4.00");

	//get <_General>
	auto _General = Hauptwerk->getChildByAttribute("ObjectType", "_General")->getChildByName("_General");

	// add <Sys_ObjectID>1</Sys_ObjectID> if necessary
	if (!_General->getChildByName("Sys_ObjectID"))
	{
		auto Sys_ObjectID = new XmlElement("Sys_ObjectID");
		Sys_ObjectID->addChildElement(_General->createTextElement("1"));
		_General->addChildElement(Sys_ObjectID);
	}

	//set minimum and current HW version to 4.0
	_General->getChildByName("Control_MinimumHauptwerkVersion")->deleteAllTextElements();
	_General->getChildByName("Control_MinimumHauptwerkVersion")->addTextElement("4.00");
	_General->getChildByName("Control_CurrentHauptwerkVersion")->deleteAllTextElements();
	_General->getChildByName("Control_CurrentHauptwerkVersion")->addTextElement("4.00");

	//add default <AudioEngine_EnablePlayingWithoutInterpolation>N</AudioEngine_EnablePlayingWithoutInterpolation>
	//todo: Change to Y?
	if (!_General->getChildByName("AudioEngine_EnablePlayingWithoutInterpolation"))
	{
		auto epwi = new XmlElement("AudioEngine_EnablePlayingWithoutInterpolation");
		epwi->addTextElement("N");
		_General->addChildElement(epwi);
	}

	//remove <MouseClickAndDragMode> from <ContinuousControl>
	auto ContinuousControl = Hauptwerk->getChildByAttribute("ObjectType", "ContinuousControl");
	for (int i = 0; i < ContinuousControl->getNumChildElements(); i++)
	{
		auto l = ContinuousControl->getChildElement(i);
		l->deleteAllChildElementsWithTagName("MouseClickAndDragMode");
	}

	//Pipe_SoundEngine01_Layer: replace some percent values by decibel values
	//and remove <...StereoBalanceAdjustPercent> adjustment options
	struct Replacement
	{
		String nameHWX;
		String nameHW4;
		bool   convertPercentToDB = false;

	};	
	std::vector<Replacement> replacements;
	replacements.push_back({ "AmpLvl_WindModelModDepthAdjustPercent", "AmpLvl_WindModelModDepthAdjustDecibels", true });
	replacements.push_back({ "AmpLvl_TremulantModDepthAdjustPercent", "AmpLvl_TremulantModDepthAdjustDecibels", true });
	replacements.push_back({ "AmpLvl_EnclosureModDepthAdjustPercent", "AmpLvl_EnclosureModDepthAdjustDecibels", true });
	replacements.push_back({ "HarmonicShaping_WindModelModDepthAdjustPercent", "HarmonicShaping_WindModelModDepthAdjustDecibelsAtThirdHarmonic", true });
	replacements.push_back({ "HarmonicShaping_TremulantModDepthAdjustPercent", "HarmonicShaping_TremulantModDepthAdjustDecibelsAtThirdHarmonic", true });
	replacements.push_back({ "EnclosureFilters_EnclosureModDepthAdjustPercent", "EnclosureFilters_EnclosureModDepthAdjustDecibels", true });
	//replacements.push_back({ "PitchLvl_WindModelModDepthAdjustPercent", "PitchLvl_WindModelModDepthAdjustPercent", true });
	//replacements.push_back({ "PitchLvl_TremulantModDepthAdjustPercent", "PitchLvl_TremulantModDepthAdjustPercent", true });
	replacements.push_back({ "AudioOut_OptimalMemoryChannelFormatCode", "AudioOut_OptimalChannelFormatCode" });

	auto Pipe_SoundEngine01_Layer = Hauptwerk->getChildByAttribute("ObjectType", "Pipe_SoundEngine01_Layer");
	for (int i = 0; i < Pipe_SoundEngine01_Layer->getNumChildElements(); i++)
	{
		//remove <...StereoBalanceAdjustPercent> adjustment options that were introduced in HW5
		auto l = Pipe_SoundEngine01_Layer->getChildElement(i);
        l->deleteAllChildElementsWithTagName("AmpLvl_StereoBalanceAdjustPercent");
		l->deleteAllChildElementsWithTagName("AmpLvl_WindModelModDepthStereoBalanceAdjustPercent");
		l->deleteAllChildElementsWithTagName("AmpLvl_TremulantModeDepthStereoBalanceAdjustPercent");
		l->deleteAllChildElementsWithTagName("AmpLvl_EnclosureModDepthStereoBalanceAdjustPercent");
		l->deleteAllChildElementsWithTagName("HarmonicShaping_ThirdAndUpperHarmonicsLevelAdjustStereoBalanceAdjustPercent");
		l->deleteAllChildElementsWithTagName("HarmonicShaping_WindModelModDepthStereoBalanceAdjustPercent");
		l->deleteAllChildElementsWithTagName("HarmonicShaping_TremulantModDepthStereoBalanceAdjustPercent");
		l->deleteAllChildElementsWithTagName("VoicingEQ01_HighFrequencyBoostStereoBalanceAdjustPercent");
		l->deleteAllChildElementsWithTagName("EnclosureFilters_EnclosureModDepthStereoBalanceAdjustPercent");

		//replace tags specified in replacements
		for (int k = 0; k < replacements.size(); k++)
		{
			auto r_HW5 = l->getChildByName(replacements[k].nameHWX);
			auto r_HW4 = l->createTextElement(replacements[k].nameHW4);
			//percent to db conversion if required
			if (replacements[k].convertPercentToDB)
			{
				double val = stod(r_HW5->getAllSubText().toStdString());
				val = Decibels::gainToDecibels(val / 100.0);
				r_HW4->setText(String(val));
			}
			l->replaceChildElement(l->getChildByName(replacements[k].nameHWX), r_HW4);
		}
	}

	Hauptwerk->writeTo(destination);
}

void showHeader()
{
	printf("******************************************************************************************************************\n");
	printf("*                                 PROSPECTUM HW5/6/7 to HW4 ODF converter v 1.25                                 *\n");
	printf("******************************************************************************************************************\n");
	printf("By     : Gernot Wurst and Christoph Schmitz, 04/2020 - 08/2022\n");
	printf("License: Creative Commons CC-BY-NC-SA-4.0, see https://creativecommons.org/licenses/by-nc-sa/4.0/ \n");
	printf("Contact: contact@prospectum.com\n");
	printf("*******************************************************************************************************************\n\n");
}

void showInstructions()
{
	printf(" Error: Wrong number of arguments! Use\n");
	printf("  1. Source file (must be in the same directory !!!)\n");
	printf("  2. Destination file (must be in the same directory !!!)\n");
	printf("  3. Optional: Add REPLACE to allow destination file to be overwritten\n\n");
	printf("  Example: ./HWxToHW4 HW567.CustomOrgan_Hauptwerk_xml HW4.CustomOrgan_Hauptwerk_xml [REPLACE] or\n");
	printf("           ./HWxToHW4 HW567.Organ_Hauptwerk_xml HW4.Organ_Hauptwerk_xml [REPLACE]\n\n");
	printf("  ATTENTION: ODFs MUST NOT BE AUTO-COMPACTED! PLEASE TURN OFF AUTO-COMPACTING IN HAUPTWERK!\n");
	printf("******************************************************************************************************************\n\n");
}

int main (int argc, char* argv[])
{
	showHeader();
	if (argc != 3 && argc != 4)
	{
		showInstructions();
		exit(1);
	}

	String fnSource(argv[1]);
	String fnDestination(argv[2]);

	String replace;
	if (argc == 4) replace = String(argv[3]);

	File source(File::getCurrentWorkingDirectory().getFullPathName() + "/" + fnSource);
	File destination(File::getCurrentWorkingDirectory().getFullPathName() + "/" + fnDestination);
	
	if (!source.existsAsFile())
	{
		printf(" Error: Source file %s does not exist!\n\n", source.getFullPathName().toStdString().c_str());
		exit(2);
	}

	if (destination.existsAsFile() && replace != "REPLACE")
	{
		printf(" Error: Destination file %s exists but must not be replaced!\n\n", destination.getFullPathName().toStdString().c_str());
		exit(3);
	}

	XmlDocument odf(source);

	auto root = odf.getDocumentElement();
	auto fileFormat = root->getStringAttribute("FileFormat");

	if (fileFormat == "Organ")
	{
		convertHWX_to_HW4NATIVE(odf, destination);
	}
	else if (fileFormat == "CustomOrgan")
	{
		convertHW5HW4CODM(odf, destination);
	}

	printf(" Conversion complete! Have fun!\n\n");
    return 0;
}
