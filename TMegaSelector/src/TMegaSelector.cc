#include "FinalStateAnalysis/TMegaSelector/interface/TMegaSelector.h"
#include "FinalStateAnalysis/TMegaSelector/interface/TMegaSelectionSet.h"
#include "FinalStateAnalysis/TMegaSelector/interface/TMegaSelectionFactory.h"

TMegaSelector::TMegaSelector(TTree* tree):
  chain(0),
  director_(tree, -1),
  factory_(new TMegaSelectionFactory(&director_)),
  filterSelection_(NULL) {}

TMegaSelector::~TMegaSelector(){
  // Cleanup selection sets
  for (SelectionSetMap::iterator i=selections_.begin(); i != selections_.end();
      ++i) {
    delete i->second;
  }
}

void TMegaSelector::Init(TTree* tree) {
  //   Set branch addresses
  if (tree == 0) return;
  chain = tree;
  director_.SetTree(chain);
  this->MegaInit(tree);
}

Bool_t TMegaSelector::Notify() {
  director_.SetTree(chain);
  return this->MegaNotify();
}

void TMegaSelector::Begin(TTree* /*deprecated*/) {
  this->MegaBegin();
}

void TMegaSelector::SlaveBegin(TTree* /*deprecated*/) {
  this->MegaSlaveBegin();
}

Bool_t TMegaSelector::Process(Long64_t entry) {
  //std::cout << "Process "  << entry << std::endl;
  allEntries_++;
  currentEntry_ = entry;
  director_.SetReadEntry(entry);
  //std::cout << "ReadEntry" << std::endl;
  //chain->GetTree()->GetEntry(entry);
  // Check if we are apply a filter
  //std::cout << "Checking Filter" << std::endl;
  if (!filterSelection_ || filterSelection_->Select()) {
    filteredEntries_++;
    return this->MegaProcess(entry);
  }
  return true;
}

void TMegaSelector::SlaveTerminate() {
  this->MegaSlaveTerminate();
}

void TMegaSelector::Terminate() {
  this->MegaTerminate();
}

void TMegaSelector::AddToSelection(const std::string& name,
    const TMegaSelection& select) {
  TMegaSelectionSet* set = GetSelectionSet(name);
  if (set == NULL) {
    // Make a new one
    set = new TMegaSelectionSet(name.c_str(), "");
    selections_[name] = set;
  }
  set->AddSelection(select);
}

void TMegaSelector::AddToSelection(const std::string& name,
    std::auto_ptr<TMegaSelection> select) {
  TMegaSelectionSet* set = GetSelectionSet(name);
  if (set == NULL) {
    // Make a new one
    set = new TMegaSelectionSet(name.c_str(), "");
    selections_[name] = set;
  }
  set->AddSelection(select);
}

TMegaSelectionSet* TMegaSelector::GetSelectionSet(const std::string& name) const
{
  SelectionSetMap::const_iterator iter = selections_.find(name);
  if (iter == selections_.end()) {
    return NULL;
  }
  else return iter->second;
}

void TMegaSelector::SetFilterSelection(const std::string& name) {
  filterSelection_ = GetSelectionSet(name);
}

unsigned int TMegaSelector::GetProcessedEntries() const {
  return allEntries_;
}

unsigned int TMegaSelector::GetFilteredEntries() const {
  return filteredEntries_;
}

const TMegaSelectionFactory* TMegaSelector::factory() const {
  return factory_.get();
}
