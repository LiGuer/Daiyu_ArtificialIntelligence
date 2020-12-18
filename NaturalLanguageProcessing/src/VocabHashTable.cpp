/*
Copyright 2013 Google Inc. 2020 LiGuer.
All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
	http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
=======================================================================*/
#include "VocabHashTable.h"
/*--------------------------------[ init ]--------------------------------*/
void VocabHashTable::init() {
	vocabHash = (int*)malloc(sizeof(int) * vocabHashSize);
	for (int i = 0; i < vocabHashSize; i++)vocabHash[i] = -1;
	vocab = (VocabHashTable_word*)realloc(vocab, vocabMemSize * sizeof(VocabHashTable_word));
};
/*--------------------------------[ Get Hash Value ]--------------------------------*/
int VocabHashTable::getNewWordHash(char* word) {
	INT64U hash = 0;
	for (int i = 0; i < strlen(word); i++) hash = hash * 37 + word[i];	//Hash function
	return hash % vocabHashSize;
}
/*--------------------------------[ ��� AddWord ]--------------------------------*/
int VocabHashTable::addWord(char* word) {
	//[0] check exist
	int index = searchWord(word);
	if (index != -1) { vocab[index].frequence++; return index; }
	//[1] save word
	unsigned int length = strlen(word) + 1 < maxStringLength ? strlen(word) + 1 : maxStringLength;
	vocab[vocabSize].word = (char*)calloc(length, sizeof(char));
	strcpy(vocab[vocabSize].word, word);
	vocab[vocabSize].frequence = 0;
	vocabSize++;
	//[2] if full, Reallocate memory if needed
	if (vocabSize + 2 >= vocabMemSize) {
		vocabMemSize += 1000;
		vocab = (VocabHashTable_word*)realloc(vocab, vocabMemSize * sizeof(VocabHashTable_word));
	}
	//[3] set Hash
	unsigned int hash = getNewWordHash(word);
	while (vocabHash[hash] != -1) { hash = (hash + 1) % vocabHashSize; } //�����λ�����У�������,���������ؿ�ͷ
	vocabHash[hash] = vocabSize - 1;
	return vocabSize - 1;
}
/*--------------------------------[ ���� Search ]--------------------------------
*	Returns position of a word in the vocabulary; if the word is not found, returns -1
**-----------------------------------------------------------------------*/
int VocabHashTable::searchWord(char* word) {
	unsigned int hash = searchWordHash(word);
	if (hash == -1)return hash;
	return vocabHash[hash];
}
int VocabHashTable::searchWordHash(char* word) {
	unsigned int hash = getNewWordHash(word);
	while (vocabHash[hash] != -1) {
		if (!strcmp(word, vocab[vocabHash[hash]].word)) return hash;
		hash = (hash + 1) % vocabHashSize;
	}return -1;
}
/*--------------------------------[ ģ������ Search Fuzzily ]--------------------------------
**-----------------------------------------------------------------------*/
void searchFuzzily(char* word, int num, char* shortlist[]) {

}
/*--------------------------------[ �洢 Save ]--------------------------------*/
void VocabHashTable::save(const char* save_vocab_file) {
	FILE* fileOut = fopen(save_vocab_file, "wb");
	for (INT64S i = 0; i < vocabHashSize; i++) {
		if (vocabHash[i] == -1)continue;
		fprintf(fileOut, "%lld\t%s\t%lld\n", i, vocab[vocabHash[i]].word, vocab[vocabHash[i]].frequence);
	}
	fclose(fileOut);
}
/*--------------------------------[ ����ʻ�� Read ]--------------------------------
*	�ļ���ȡ�ʻ㣬����Hash��
*	[1]ReadWord [2]Vocab_AddWord [3]Vocab_Sort
**-----------------------------------------------------------------------*/
void VocabHashTable::readVocab(char* read_vocab_file) {
	FILE* fin = fopen(read_vocab_file, "rb");
	if (fin == NULL) { printf("VocabHashTable file not found\n"); exit(-1); }
	init();
	char word[maxStringLength];
	int hash;
	while (true) {
		if (feof(fin)) break;					//feof()������ϵ��ļ����������ļ����������ط�0���ļ�δ����������0ֵ
		fscanf(fin, "%lld\t%c\t%lld",&hash, &word, &vocab[vocabSize].frequence);			//�ôʼ���Hash�� //��ȡ��¼����
		// save word to VocabHashTable
		vocab[vocabSize].word = (char*)calloc(strlen(word), sizeof(char));
		strcpy(vocab[vocabSize++].word, word);
		// if full, Reallocate memory if needed
		if (vocabSize + 2 >= vocabMemSize) {
			vocabMemSize += 1000;
			vocab = (VocabHashTable_word*)realloc(vocab, vocabMemSize * sizeof(VocabHashTable_word));
		}
		vocabHash[hash] = vocabSize - 1;
	}
}
/*--------------------------------[ �����Ƶ�� ]--------------------------------
*	Reduces the vocabulary by removing infrequent tokens
*	min_reduce:��Ƶ�������ֵ
**-----------------------------------------------------------------------*/
void VocabHashTable::reduceInfrequentWord(int threshold) {
	//[1] if Infrequent word, delete. else save forward 
	int b = 0;
	for (int i = 0; i < vocabSize; i++)
		if (vocab[i].frequence > threshold) {
			vocab[b].frequence = vocab[i].frequence;
			vocab[b].word = vocab[i].word;
			b++;
		}
		else free(vocab[i].word);
	vocabSize = b;
	//[2] re-computed Hash, as it is not actual
	for (int i = 0; i < vocabHashSize; i++) vocabHash[i] = -1;
	for (int i = 0; i < vocabSize; i++) {
		unsigned int hash = getNewWordHash(vocab[i].word);
		while (vocabHash[hash] != -1) hash = (hash + 1) % vocabHashSize;
		vocabHash[hash] = i;
	}
}
/*--------------------------------[ ���� ]--------------------------------*/
#include<algorithm>
void VocabHashTable::sort(VocabHashTable_word* begin, VocabHashTable_word* end, bool(*cmd)(VocabHashTable_word a, VocabHashTable_word b)) {
	std::sort(&vocab[1], &vocab[vocabSize], cmd);
	//[2] re-computed Hash, as it is not actual
	for (int i = 0; i < vocabHashSize; i++) vocabHash[i] = -1;
	for (int i = 0; i < vocabSize; i++) {
		unsigned int hash = getNewWordHash(vocab[i].word);
		while (vocabHash[hash] != -1) hash = (hash + 1) % vocabHashSize;
		vocabHash[hash] = i;
	}
}
/*--------------------------------[ get Word From Text ]--------------------------------*/
void VocabHashTable::getWordFromText(const char* url) {
	FILE* fin = fopen(url, "r");
	char word[maxStringLength];
	int wordLength = 0;
	while (!feof(fin)) {
		char ch = fgetc(fin);
		if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
		if (ch >= 'a' && ch <= 'z') {
			word[wordLength++] = ch;
			if (wordLength >= maxStringLength - 1) wordLength--;   // Truncate too long words
			continue;
		}
		if (wordLength == 0)continue;
		word[wordLength++] = '\0';
		char* word0 = (char*)calloc(wordLength, sizeof(char));
		memcpy(word0, word, sizeof(char) * wordLength);
		addWord(word0);
		wordLength = 0;
	}
}