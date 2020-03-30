/*
  TSynth patch saving and recall works like an analogue polysynth from the late 70s (Prophet 5).
  When you recall a patch, all the front panel controls will be different values from those saved in the patch. Moving them will cause a jump to the current value.

  BACK
  Cancels current mode such as save, recall, delete and rename patches

  RECALL
  Recall shows list of patches. Use encoder to move through list.
  Enter button on encoder chooses highlighted patch or press Recall again.
  Recall also recalls the current patch settings if the panel controls have been altered.
  Holding Recall for 1.5s will initialise the synth with all the current panel control settings - the synth sounds the same as the controls are set.


  SAVE
  Save will save the current settings to a new patch at the end of the list or you can use the encoder to overwrite an existing patch.
  Press Save again to save it. If you want to name/rename the patch, press the encoder enter button and use the encoder and enter button to choose an alphanumeric name.
  Holding Save for 1.5s will go into a patch deletion mode. Use encoder and enter button to choose and delete patch. Patch numbers will be changed on the SD card to be consecutive again.
*/
//Agileware CircularBuffer available in libraries manager
#include <CircularBuffer.h>

#define TOTALCHARS 63

const char CHARACTERS[TOTALCHARS] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' ', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
int charIndex = 0;
char currentCharacter = 0;
String renamedPatch = "";

struct PatchNoAndName
{
  int patchNo;
  String patchName;
};

CircularBuffer<PatchNoAndName, PATCHES_LIMIT> patches;

size_t readField(File *file, char *str, size_t size, const char *delim)
{
  char ch;
  size_t n = 0;
  while ((n + 1) < size && file->read(&ch, 1) == 1)
  {
    // Delete CR.
    if (ch == '\r')
    {
      continue;
    }
    str[n++] = ch;
    if (strchr(delim, ch))
    {
      break;
    }
  }
  str[n] = '\0';
  return n;
}

void recallPatchData(File patchFile, String data[])
{
  //Read patch data from file and set current patch parameters
  size_t n;     // Length of returned field with delimiter.
  char str[20]; // Must hold longest field with delimiter and zero byte.
  int i = 0;
  while (patchFile.available() && i < NO_OF_PARAMS)
  {
    n = readField(&patchFile, str, sizeof(str), ",\n");
    // done if Error or at EOF.
    if (n == 0)
      break;
    // Print the type of delimiter.
    if (str[n - 1] == ',' || str[n - 1] == '\n')
    {
      //        Serial.print(str[n - 1] == ',' ? F("comma: ") : F("endl:  "));
      // Remove the delimiter.
      str[n - 1] = 0;
    }
    else
    {
      // At eof, too long, or read error.  Too long is error.
      Serial.print(patchFile.available() ? F("error: ") : F("eof:   "));
    }
    // Print the field.
    //    Serial.print(i);
    //    Serial.print(" - ");
    //    Serial.println(str);
    data[i++] = String(str);
  }
}

void getPatches(File file)
{
  patches.clear();
  while (true)
  {
    String data[NO_OF_PARAMS]; //Array of data read in
    File patchFile = file.openNextFile();
    if (!patchFile)
    {
      break;
    }
    if (patchFile.isDirectory())
    {
      Serial.println("Ignoring Dir");
    }
    else
    {
      recallPatchData(patchFile, data);
      patches.push(PatchNoAndName{atoi(patchFile.name()), data[0]});
      Serial.println(String(patchFile.name()) + ":" + data[0]);
    }
    patchFile.close();
  }
}

void deletePatch(const char *patchNo)
{
  //Don't delete last patch
  if (SD.exists(patchNo) && patches.size() > 1)
  {
    SD.remove(patchNo);
    //TODO renumber filenames on SD card
  }
}

void savePatch(const char *patchNo, String patchData)
{
  Serial.print("savePatch Patch No:");
  Serial.println(patchNo);
  //Overwrite existing patch by deleting
  if (SD.exists(patchNo))
  {
    SD.remove(patchNo);
  }
  File patchFile = SD.open(patchNo, FILE_WRITE);
  if (patchFile)
  {
    Serial.print("Writing Patch No:");
    Serial.println(patchNo);
    Serial.println(patchData);
    patchFile.println(patchData);
    patchFile.close();
  }
  else
  {
    Serial.print("Error writing Patch file:");
    Serial.println(patchNo);
  }
}

void savePatch(const char *patchNo, String patchData[])
{
  String dataString = patchData[0];
  for (int i = 1; i < NO_OF_PARAMS; i++)
  {
    dataString = dataString + "," + patchData[i];
  }
  savePatch(patchNo, dataString);
}


int compare(const void *a, const void *b) {
  return ( *(int*)a - * (int*)b );
}


//// A utility function to swap two elements
//void swap(int* a, int* b)
//{
//  int t = *a;
//  *a = *b;
//  *b = t;
//}
//
///* This function takes last element as pivot, places
//   the pivot element at its correct position in sorted
//    array, and places all smaller (smaller than pivot)
//   to left of pivot and all greater elements to right
//   of pivot */
//int partition (CircularBuffer<PatchNoAndName, PATCHES_LIMIT> arr, int low, int high)
//{
//  int pivot = arr[high].patchNo;    // pivot
//  int i = (low - 1);  // Index of smaller element
//
//  for (int j = low; j <= high - 1; j++)
//  {
//    // If current element is smaller than the pivot
//    if (arr[j].patchNo < pivot)
//    {
//      i++;    // increment index of smaller element
//      swap(&arr[i], &arr[j]);
//    }
//  }
//  swap(&arr[i + 1], &arr[high]);
//  return (i + 1);
//}
//
///* The main function that implements QuickSort
//  arr[] --> Array to be sorted,
//  low  --> Starting index,
//  high  --> Ending index */
//void quickSort(CircularBuffer<PatchNoAndName, PATCHES_LIMIT> arr, int low, int high)
//{
//  if (low < high)
//  {
//    /* pi is partitioning index, arr[p] is now
//       at right place */
//    int pi = partition(arr, low, high);
//
//    // Separately sort elements before
//    // partition and after partition
//    quickSort(arr, low, pi - 1);
//    quickSort(arr, pi + 1, high);
//  }
//}





void sortPatches()
{
   // quickSort(arr, 0, patches.size()-1); 
  
  //  //Sort patches buffer to be consecutive ascending patchNo order
  //  int arrayToSort[patches.size()];
  //  for (int i = 0; i < patches.size() - 1; ++i)
  //  {
  //    arrayToSort[i] = patches[i].patchNo;
  //  }
  //  qsort(arrayToSort, patches.size(), sizeof(int), compare);
  //
  //  for (int i = 0; i < patches.size() - 1; ++i)
  //  {
  //
  //    for (int j = 0; j < patches.size() - 1; ++j)
  //    {
  //if(patches[j].patchNo == arrayToSort[i]){
  //  patches[j].
  //  break;
  //}
  //    }
  //  }

  //  for (int i = 1; i < patches.size(); ++i)
  //  {
  //    String data[NO_OF_PARAMS]; //Array of data read in
  //    File file = SD.open(String(patches[i].patchNo).c_str());
  //    recallPatchData(file, data);
  //    file.close();
  //    savePatch(String(i + 1).c_str(), data);
  //  }
  //deletePatch(String(patches.size() + 1).c_str()); //delete final patch still on SD
}
