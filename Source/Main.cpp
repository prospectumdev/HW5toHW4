#include <JuceHeader.h>

void convertHW5HW4CODM(File source, File destination)
{
	XmlDocument odf(source);
	auto root = odf.getDocumentElement();

	//TODO: Check if auto-compacted. If true, refuse to process and report error

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

	root->writeTo(destination);
}

void convertHWX_to_HW4NATIVE(File source, File destination)
{
	XmlDocument odf(source);
	auto Hauptwerk = odf.getDocumentElement();

	//TODO: Check if auto-compacted. If true, refuse to process and report error

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

	auto ContinuousControl = Hauptwerk->getChildByAttribute("ObjectType", "ContinuousControl");
	for (int i = 0; i < ContinuousControl->getNumChildElements(); i++)
	{
		auto l = ContinuousControl->getChildElement(i);
		l->deleteAllChildElementsWithTagName("MouseClickAndDragMode");
	}

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

	Hauptwerk->writeTo(destination);
}

void showHeader()
{
	printf("******************************************************************************************************************\n");
	printf("*                                 PROSPECTUM HW7/6/5 to HW4 ODF converter v 1.02                                   *\n");
	printf("******************************************************************************************************************\n");
	printf("By     : Gernot Wurst and Christoph Schmitz, 04/2020\n");
	printf("License: Creative Commons CC-BY-NC-SA-4.0, see https://creativecommons.org/licenses/by-nc-sa/4.0/ \n");
	printf("Contact: contact@prospectum.com\n");
	printf("*******************************************************************************************************************\n\n");
}

void showInstructions()
{
	printf(" Error: Wrong number of arguments! Use\n");
	printf("  1. Conversion mode. Values are CODM and NATIVE(HW7/6/5 are supported)\n");
	printf("  2. Source file (must be in the same directory!!!)\n");
	printf("  3. Destination file (must be in the same directory!!!)\n");
	printf("  4. Optional: Add REPLACE to allow destination file to be overwritten\n\n");
	printf("  Example: HWXtoHW4.exe CODM ExampleOrgan1HW5.CustomOrgan_Hauptwerk_xml ExampleOrgan1HW4.CustomOrgan_Hauptwerk_xml\n\n");
	printf("  ATTENTION: ODFs MUST NOT BE AUTO-COMPACTED! PLEASE TURN OFF AUTO-COMPACTING IN HAUPTWERK!");
	printf("******************************************************************************************************************\n\n");
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
	if (argc == 5) replace = String(argv[4]);

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

	if (mode == "CODM")		   convertHW5HW4CODM(source, destination);
	else if (mode == "NATIVE") convertHWX_to_HW4NATIVE(source, destination);
	else
	{
		printf(" Error: Mode is %s but must be CODM or NATIVE!\n\n",mode.toStdString().c_str());
		exit(4);
	}

	printf(" Conversion complete! Have fun!\n\n");
    return 0;
}
