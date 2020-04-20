#include <JuceHeader.h>

void convertHW5HW4CODM(String fnSource, String fnDestination)
{
	XmlDocument odf(fnSource);
	auto root = odf.getDocumentElement();

	root->setAttribute("FileFormatVersion", "4.00");

	auto _General = root->getChildByAttribute("ObjectType", "_General");
	_General = _General->getChildByName("_General");

	auto Sys_ObjectID = _General->getChildByName("Sys_ObjectID");
	if (!Sys_ObjectID)
	{
		Sys_ObjectID = new XmlElement("Sys_ObjectID");
		Sys_ObjectID->addChildElement(_General->createTextElement("1"));
		_General->addChildElement(Sys_ObjectID);
	}

	root->writeTo(fnDestination);
}

void convertHW5HW4NATIVE(String fnSource, String fnDestination)
{
	XmlDocument odf(fnSource);
	auto Hauptwerk = odf.getDocumentElement();

	Hauptwerk->setAttribute("FileFormatVersion", "4.00");

	auto _General = Hauptwerk->getChildByAttribute("ObjectType", "_General");
	_General = _General->getChildByName("_General");

	if (!_General->getChildByName("Sys_ObjectID"))
	{
		auto Sys_ObjectID = new XmlElement("Sys_ObjectID");
		Sys_ObjectID->addChildElement(_General->createTextElement("1"));
		_General->addChildElement(Sys_ObjectID);
	}

	_General->getChildByName("Control_MinimumHauptwerkVersion")->deleteAllTextElements();
	_General->getChildByName("Control_MinimumHauptwerkVersion")->addTextElement("4.00");
	_General->getChildByName("Control_CurrentHauptwerkVersion")->deleteAllTextElements();
	_General->getChildByName("Control_CurrentHauptwerkVersion")->addTextElement("4.00");

	struct Replacement
	{
		String nameHW5;
		String nameHW4;
		bool   convertPercentToDB = false;

	};
	std::vector<Replacement> rep;
	rep.push_back({ "AudioOut_OptimalMemoryChannelFormatCode", "AudioOut_OptimalMemoryChannelFormatCode" });
	rep.push_back({ "AmpLvl_WindModelModDepthAdjustPercent", "AmpLvl_WindModelModDepthAdjustPercent", true });
	rep.push_back({ "AmpLvl_TremulantModDepthAdjustPercent", "AmpLvl_TremulantModDepthAdjustPercent", true });
	rep.push_back({ "AmpLvl_EnclosureModDepthAdjustPercent", "AmpLvl_EnclosureModDepthAdjustPercent", true });
	rep.push_back({ "PitchLvl_WindModelModDepthAdjustPercent", "PitchLvl_WindModelModDepthAdjustPercent", true });
	rep.push_back({ "PitchLvl_TremulantModDepthAdjustPercent", "PitchLvl_TremulantModDepthAdjustPercent", true });
	rep.push_back({ "EnclosureFilters_EnclosureModDepthAdjustPercent", "EnclosureFilters_EnclosureModDepthAdjustPercent", true });
	rep.push_back({ "HarmonicShaping_WindModelModDepthAdjustPercent", "HarmonicShaping_WindModelModDepthAdjustDecibelsAtThirdHarmonic", true });
	rep.push_back({ "HarmonicShaping_TremulantModDepthAdjustPercent", "HarmonicShaping_TremulantModDepthAdjustDecibelsAtThirdHarmonic", true });

	auto Pipe_SoundEngine01_Layer = Hauptwerk->getChildByAttribute("ObjectType", "Pipe_SoundEngine01_Layer");
	for (int i = 0; i < Pipe_SoundEngine01_Layer->getNumChildElements(); i++)
	{
		auto l = Pipe_SoundEngine01_Layer->getChildElement(i);
		l->deleteAllChildElementsWithTagName("AmpLvl_WindModelModDepthStereoBalanceAdjustPercent");
		l->deleteAllChildElementsWithTagName("AmpLvl_TremulantModeDepthStereoBalanceAdjustPercent");
		l->deleteAllChildElementsWithTagName("AmpLvl_EnclosureModDepthStereoBalanceAdjustPercent");
		l->deleteAllChildElementsWithTagName("AmpLvl_StereoBalanceAdjustPercent");
		l->deleteAllChildElementsWithTagName("HarmonicShaping_ThirdAndUpperHarmonicsLevelAdjustStereoBalanceAdjustPercent");
		l->deleteAllChildElementsWithTagName("HarmonicShaping_WindModelModDepthStereoBalanceAdjustPercent");
		l->deleteAllChildElementsWithTagName("HarmonicShaping_TremulantModDepthStereoBalanceAdjustPercent");
		l->deleteAllChildElementsWithTagName("VoicingEQ01_HighFrequencyBoostStereoBalanceAdjustPercent");
		l->deleteAllChildElementsWithTagName("EnclosureFilters_EnclosureModDepthStereoBalanceAdjustPercent");

		for (int k = 0; k < rep.size(); k++)
		{
			auto r_HW5 = l->getChildByName(rep[k].nameHW5);
			auto r_HW4 = l->createTextElement(rep[k].nameHW4);
			if (rep[k].convertPercentToDB)
			{
				double val = stod(r_HW5->getAllSubText().toStdString());
				val = Decibels::gainToDecibels(val / 100.0);
				r_HW4->setText(String(val));
			}
			l->replaceChildElement(l->getChildByName(rep[k].nameHW5), r_HW4);
		}
	}

	Hauptwerk->writeTo(fnDestination);
}

void showHeader()
{
	printf("------------------------------------------------------------------------------------------------------------------\n\n");
	printf(">>> PROSPECTUM HW5 to HW4 ODF converter v 1.0 <<<\n\n");
	printf("By Gernot Wurst and Michael Schmitz, 04/2020\n\n");
	printf("License: Creative Commons CC-BY-NC-SA-4.0, see\n");
	printf("https://creativecommons.org/licenses/by-nc-sa/4.0/ \n\n");
	printf("Contact: contact@prospectum.com");
	printf("------------------------------------------------------------------------------------------------------------------\n\n");
}

void showInstructions()
{
	printf(" Error: Wrong number of arguments! use\n\n");
	printf("  1. Conversion mode. Values are CODM and NATIVE\n");
	printf("  2. Source file (full path)\n");
	printf("  3. Destination file (full path)\n");
	printf("  4. Optional: Add REPLACE to allow destination file to be overwritten\n\n");
	printf("  Example: HW5toHW4.exe CODM ExampleOrgan1HW5.CustomOrgan_Hauptwerk_xml ExampleOrgan1HW4.CustomOrgan_Hauptwerk_xml\n\n");
	printf("------------------------------------------------------------------------------------------------------------------\n\n");
}

int main (int argc, char* argv[])
{
	showHeader();
	if (argc != 4 && argc != 5)
	{
		showInstructions();
		exit(1);
	}

	String mode(argv[1]);
	String fnSource(argv[2]);
	String fnDestination(argv[3]);
	String replace;
	if (argc == 4) replace = String(argv[4]);

	if (File(fnSource).existsAsFile())
	{
		printf(" Error: Source file does not exist!\n\n");
		exit(2);
	}

	if (File(fnDestination).existsAsFile() && replace != "REPLACE")
	{
		printf(" Error: Destination file exists but must not be replaces!\n\n");
		exit(3);
	}

	if (mode == "CODM")		   convertHW5HW4CODM (fnSource, fnDestination);
	else if (mode == "NATIVE") convertHW5HW4NATIVE(fnSource, fnDestination);
	else
	{
		printf(" Error: Mode is %s but must be CODM or NATIVE!\n\n",mode.toStdString().c_str());
		exit(4);
	}

	printf(" Conversion complete! Have fun!\n\n");
    return 0;
}
