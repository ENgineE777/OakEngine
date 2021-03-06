
#include "MetaData.h"
#include "imgui.h"
#include "Root/Root.h"
#include "Transform.h"
#include "Root/Scenes/SceneEntity.h"

#ifdef OAK_EDITOR
#include "Editor/Editor.h"
#endif

namespace Oak
{
	void MetaData::Prepare(void* set_owner, void* set_root)
	{
		if (!inited)
		{
			Init();
			inited = true;
		}

		owner = set_owner;
		root = set_root ? set_root : owner;

		#ifdef OAK_EDITOR
		StringUtils::Printf(guiID, 64, "%p", owner);
		#endif

		for (int i = 0; i < properties.size(); i++)
		{
			Property& prop = properties[i];
			prop.value = (uint8_t*)owner + prop.offset;

			#ifdef OAK_EDITOR
			if (prop.type == Type::Callback)
			{
				prop.value = (uint8_t*)prop.callback;
			}
			#endif

			if (prop.adapter)
			{
				prop.adapter->value = prop.value;
			}
		}
	}

	void MetaData::SetDefValues()
	{
		for (int i = 0; i < properties.size(); i++)
		{
			Property& prop = properties[i];

			if (prop.type == Type::Boolean)
			{
				memcpy(prop.value, &prop.defvalue.boolean, sizeof(bool));
			}
			else
			if (prop.type == Type::Integer)
			{
				memcpy(prop.value, &prop.defvalue.integer, sizeof(int));
			}
			else
			if (prop.type == Type::Float)
			{
				memcpy(prop.value, &prop.defvalue.flt, sizeof(float));
			}
			else
			if (prop.type == Type::String || prop.type == Type::FileName)
			{
				*((eastl::string*)prop.value) = defStrings[prop.defvalue.string];
			}
			else
			if (prop.type == Type::Color)
			{
				memcpy(prop.value, prop.defvalue.color, sizeof(float) * 4);
			}
			else
			if (prop.type == Type::Enum)
			{
				MetaDataEnum& enm = enums[prop.defvalue.enumIndex];

				int value = enm.values[enm.defIndex];
				memcpy(prop.value, &value, sizeof(int));
			}
			else
			if (prop.type == Type::EnumString)
			{
				*((eastl::string*)prop.value) = "";
			}
			else
			if (prop.type == Type::AssetTexture)
			{
				AssetTextureRef* ref = reinterpret_cast<AssetTextureRef*>(prop.value);
				ref->ReleaseRef();
			}
			else
			if (prop.type == Type::AssetAnimGraph2D)
			{
				AssetAnimGraph2DRef* ref = reinterpret_cast<AssetAnimGraph2DRef*>(prop.value);
				ref->ReleaseRef();
			}
			else
			if (prop.type == Type::Transform)
			{
				Transform* transform = (Transform*)prop.value;
				transform->position = 0.0f;
				transform->rotation = 0.0f;
				transform->scale = 1.0f;
				transform->offset = 0.5f;
			}
		}
	}

	void MetaData::Load(JsonReader& reader)
	{
		for (int i = 0; i < properties.size(); i++)
		{
			Property& prop = properties[i];

			if (prop.type == Type::Boolean)
			{
				bool val;
				if (reader.Read(prop.name.c_str(), val))
				{
					memcpy(prop.value, &val, sizeof(bool));
				}
			}
			else
			if (prop.type == Type::Integer || prop.type == Type::Enum)
			{
				int val;
				if (reader.Read(prop.name.c_str(), val))
				{
					memcpy(prop.value, &val, sizeof(int));
				}
			}
			else
			if (prop.type == Type::Float)
			{
				float val;
				if (reader.Read(prop.name.c_str(), val))
				{
					memcpy(prop.value, &val, sizeof(float));
				}
			}
			else
			if (prop.type == Type::String || prop.type == Type::EnumString || prop.type == Type::FileName)
			{
				reader.Read(prop.name.c_str(), *((eastl::string*)prop.value));
			}
			else
			if (prop.type == Type::Color)
			{
				reader.Read(prop.name.c_str(), *((Oak::Color*)prop.value));
			}
			else
			if (prop.type == Type::AssetTexture)
			{
				AssetTextureRef* ref = reinterpret_cast<AssetTextureRef*>(prop.value);
				ref->LoadData(reader, prop.name.c_str());
			}
			else
			if (prop.type == Type::AssetAnimGraph2D)
			{
				AssetAnimGraph2DRef* ref = reinterpret_cast<AssetAnimGraph2DRef*>(prop.value);
				ref->LoadData(reader, prop.name.c_str());
			}
			else
			if (prop.type == Type::Transform)
			{
				Transform* transform = (Transform*)prop.value;
				transform->Load(reader, prop.propName.c_str());
			}
			else
			if (prop.type == Type::SceneEntity)
			{
				if (reader.EnterBlock(prop.propName.c_str()))
				{
					SceneEntityRef* ref = (SceneEntityRef*)prop.value;

					reader.Read("uid", ref->uid);
					
					reader.LeaveBlock();
				}
			}
			else
			if (prop.type == Type::Array)
			{
				if (reader.EnterBlock(prop.name.c_str()))
				{
					int count = 0;
					if (reader.Read("count", count))
					{
						prop.adapter->Resize(count);
				
						for (int i = 0; i < count; i++)
						{
							reader.EnterBlock("Elem");

							prop.adapter->GetMetaData()->Prepare(prop.adapter->GetItem(i), root);
							prop.adapter->GetMetaData()->Load(reader);

							reader.LeaveBlock();
						}
					}

					reader.LeaveBlock();
				}
			}
		}
	}

	void MetaData::PostLoad(Scene* scene)
	{
		for (auto& prop : properties)
		{
			if (prop.type == Type::Array)
			{
				for (int i = 0; i < prop.adapter->GetSize(); i++)
				{
					prop.adapter->GetMetaData()->Prepare(prop.adapter->GetItem(i), root);
					prop.adapter->GetMetaData()->PostLoad(scene);
				}
			}
			else
			if (prop.type == Type::SceneEntity)
			{
				SceneEntityRef* ref = (SceneEntityRef*)prop.value;
				ref->entity = scene->FindEntity(ref->uid);
			}
		}
	}

	void MetaData::Save(JsonWriter& writer)
	{
		for (int i = 0; i < properties.size(); i++)
		{
			Property& prop = properties[i];

			if (prop.type == Type::Boolean)
			{
				writer.Write(prop.name.c_str(), *((bool*)prop.value));
			}
			else
			if (prop.type == Type::Integer || prop.type == Type::Enum)
			{
				writer.Write(prop.name.c_str(), *((int*)prop.value));
			}
			else
			if (prop.type == Type::Float)
			{
				writer.Write(prop.name.c_str(), *((float*)prop.value));
			}
			else
			if (prop.type == Type::String || prop.type == Type::EnumString || prop.type == Type::FileName)
			{
				writer.Write(prop.name.c_str(), ((eastl::string*)prop.value)->c_str());
			}
			else
			if (prop.type == Type::Color)
			{
				writer.Write(prop.name.c_str(), *((Oak::Color*)prop.value));
			}
			else
			if (prop.type == Type::AssetTexture)
			{
				AssetTextureRef* ref = reinterpret_cast<AssetTextureRef*>(prop.value);
				ref->SaveData(writer, prop.name.c_str());
			}
			else
			if (prop.type == Type::AssetAnimGraph2D)
			{
				AssetAnimGraph2DRef* ref = reinterpret_cast<AssetAnimGraph2DRef*>(prop.value);
				ref->SaveData(writer, prop.name.c_str());
			}
			else
			if (prop.type == Type::Transform)
			{
				Transform* transform = (Transform*)prop.value;
				transform->Save(writer, prop.propName.c_str());
			}
			else
			if (prop.type == Type::SceneEntity)
			{
				SceneEntityRef* ref = (SceneEntityRef*)prop.value;

				writer.StartBlock(prop.propName.c_str());

				writer.Write("uid", ref->uid);

				writer.FinishBlock();
			}
			else
			if (prop.type == Type::Array)
			{
				writer.StartBlock(prop.name.c_str());

				int count = prop.adapter->GetSize();
				writer.Write("count", count);
			
				writer.StartArray("Elem");

				for (int i = 0; i < count; i++)
				{
					writer.StartBlock(nullptr);

					prop.adapter->GetMetaData()->Prepare(prop.adapter->GetItem(i), root);
					prop.adapter->GetMetaData()->Save(writer);

					writer.FinishBlock();
				}

				writer.FinishArray();

				writer.FinishBlock();
			}
		}
	}

	void MetaData::Copy(void* source, eastl::vector<Property>& sourceProperties)
	{
		for (auto& prop : properties)
		{
			uint8_t* src = nullptr;

			for (auto& sourceProp : sourceProperties)
			{
				if (StringUtils::IsEqual(prop.name.c_str(), sourceProp.name.c_str()))
				{
					src = (uint8_t*)source + sourceProp.offset;
					break;
				}
			}

			if (src == nullptr)
			{
				continue;
			}

			if (prop.type == Type::Boolean)
			{
				memcpy(prop.value, src, sizeof(bool));
			}
			else
			if (prop.type == Type::Integer || prop.type == Type::Enum)
			{
				memcpy(prop.value, src, sizeof(int));
			}
			else
			if (prop.type == Type::Float)
			{
				memcpy(prop.value, src, sizeof(float));
			}
			else
			if (prop.type == Type::String || prop.type == Type::EnumString || prop.type == Type::FileName)
			{
				*((eastl::string*)prop.value) = *((eastl::string*)src);
			}
			else
			if (prop.type == Type::Color)
			{
				memcpy(prop.value, src, sizeof(float) * 4);
			}
			else
			if (prop.type == Type::AssetTexture)
			{
				memcpy(prop.value, src, sizeof(AssetTextureRef));
			}
			else
			if (prop.type == Type::AssetAnimGraph2D)
			{
				memcpy(prop.value, src, sizeof(AssetAnimGraph2DRef));
			}
			else
			if (prop.type == Type::Transform)
			{
				Transform* transformSrc = (Transform*)src;
				Transform* transformDest = (Transform*)prop.value;

				transformDest->position = transformSrc->position;
				transformDest->rotation = transformSrc->rotation;
				transformDest->scale = transformSrc->scale;
				transformDest->size = transformSrc->size;
				transformDest->offset = transformSrc->offset;
			}
			else
			if (prop.type == Type::SceneEntity)
			{
				memcpy(prop.value, src, sizeof(SceneEntityRef));
			}
			else
			if (prop.type == Type::Array)
			{
				prop.adapter->value = src;
				int count = prop.adapter->GetSize();
			
				prop.adapter->value = prop.value;
				prop.adapter->Resize(count);

				for (int i = 0; i < count; i++)
				{
					prop.adapter->value = src;
					uint8_t* srcItem = prop.adapter->GetItem(i);
					auto& srcProperties = prop.adapter->GetMetaData()->properties;

					prop.adapter->value = prop.value;

					prop.adapter->GetMetaData()->Prepare(prop.adapter->GetItem(i), root);
					prop.adapter->GetMetaData()->Copy(srcItem, srcProperties);
				}
			}
		}
	}

	#ifdef OAK_EDITOR
	void MetaData::ConstructCategoriesData()
	{
		for (int i = 0; i < properties.size(); i++)
		{
			int index = -1;

			for (int j = 0; j < categoriesData.size(); j++)
			{
				if (StringUtils::IsEqual(categoriesData[j].name.c_str(), properties[i].catName.c_str()))
				{
					index = j;
					break;
				}
			}

			if (index == -1)
			{
				CategoryData data;
				data.name = properties[i].catName;
				data.indices.push_back(i);

				categoriesData.push_back(data);
			}
			else
			{
				categoriesData[index].indices.push_back(i);
			}
		}
	}

	bool MetaData::ImGuiVector(float* x, float* y, float* z, float* w, const char* name, const char* propID)
	{
		bool changed = false;

		ImGui::Text(name);
		ImGui::NextColumn();

		float* values[] = { x, y, z, w };
		const char* prefix[] = { "x", "y", "z", "w" };

		float width = 0.0f;

		for (int i = 0; i < 4; i++)
		{
			if (values[i])
			{
				width += 1.0f;
			}
		}

		width = ImGui::GetContentRegionAvail().x / width;

		bool firstEntry = true;

		for (int i = 0; i < 4; i++)
		{
			if (values[i])
			{
				if (!firstEntry)
				{
					ImGui::SameLine();
				}

				ImGui::SetNextItemWidth(width);

				char propGuiID[256];
				StringUtils::Printf(propGuiID, 256, "%s%s%s", propID, name, prefix[i]);

				if (ImGui::InputFloat(propGuiID, values[i]))
				{
					changed = true;
				}

				firstEntry = false;
			}
		}

		ImGui::NextColumn();

		return changed;
	}

	void MetaData::ImGuiWidgets()
	{
		if (categoriesData.size() == 0)
		{
			ConstructCategoriesData();
		}

		char propGuiID[256];

		for (int j = 0; j < categoriesData.size(); j++)
		{
			StringUtils::Printf(propGuiID, 256, "%s###%sCat%i", categoriesData[j].name.c_str(), guiID, j);

			if (root == owner)
			{
				ImGui::Columns(1);
			}

			bool is_open = ImGui::CollapsingHeader(propGuiID, ImGuiTreeNodeFlags_DefaultOpen);
			
			if (root == owner)
			{
				ImGui::Columns(2);
			}
			else
			{
				ImGui::NextColumn();

				if (ImGui::CollapsingHeader(propGuiID))
				{

				}

				ImGui::NextColumn();
			}

			if (is_open)
			{
				for (int i = 0; i < categoriesData[j].indices.size(); i++)
				{
					auto& prop = properties[categoriesData[j].indices[i]];

					if (prop.type != Type::FileName && prop.type != Type::Callback && prop.type != Type::Array)
					{
						StringUtils::Printf(propGuiID, 256, "###%s%s%i", categoriesData[j].name.c_str(), guiID, i);
					}

					if (prop.type == Type::Callback)
					{
						ImGui::Columns(1);

						StringUtils::Printf(propGuiID, 256, "%s%s%i", prop.propName.c_str(), propGuiID, i);

						if (ImGui::Button(propGuiID, ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
						{
							prop.callback(owner);
						}

						ImGui::NextColumn();

						ImGui::Columns(2);
					}
					else
					if (prop.type == Type::Array)
					{
						StringUtils::Printf(propGuiID, 256, "%s###%s%i", prop.propName.c_str(), guiID, i);
						bool items_open = ImGui::TreeNode(propGuiID);

						StringUtils::Printf(propGuiID, 256, "Add###%s%s%iAdd", categoriesData[j].name.c_str(), guiID, i);

						ImGui::NextColumn();

						if (ImGui::Button(propGuiID, ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
						{
							prop.adapter->PushBack();
							prop.adapter->GetMetaData()->Prepare(prop.adapter->GetItem((int)prop.adapter->GetSize() - 1));
							prop.adapter->GetMetaData()->SetDefValues();
							prop.changed = true;
						}

						ImGui::NextColumn();

						if (items_open)
						{
							int count = prop.adapter->GetSize();
							int index2delete = -1;

							for (int i = 0; i < count; i++)
							{
								bool itemOpen = ImGui::TreeNode(StringUtils::PrintTemp("Item %i###%s%i", i, guiID, i));

								ImGui::NextColumn();

								StringUtils::Printf(propGuiID, 256, "Del###%s%s%iDel", categoriesData[j].name.c_str(), guiID, i);

								if (ImGui::Button(propGuiID, ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
								{
									index2delete = i;
								}

								ImGui::NextColumn();

								if (itemOpen)
								{
									prop.adapter->GetMetaData()->Prepare(prop.adapter->GetItem(i), root);
									prop.adapter->GetMetaData()->ImGuiWidgets();

									ImGui::TreePop();
								}
							}

							if (index2delete != -1)
							{
								prop.adapter->Delete(index2delete);
								prop.changed = true;
							}

							ImGui::TreePop();
						}
					}
					else
					if (prop.type == Type::Transform)
					{
						Transform* transform = (Transform*)prop.value;

						if (transform->transformFlag & TransformFlag::MoveXYZ)
						{
							ImGuiVector(transform->transformFlag & TransformFlag::MoveX ? &transform->position.x : nullptr,
										transform->transformFlag & TransformFlag::MoveY ? &transform->position.y : nullptr,
										transform->transformFlag & TransformFlag::MoveZ ? &transform->position.z : nullptr,
										nullptr, "Position", propGuiID);
						}

						if (transform->transformFlag & TransformFlag::RotateXYZ)
						{
							ImGuiVector(transform->transformFlag & TransformFlag::RotateX ? &transform->rotation.x : nullptr,
										transform->transformFlag & TransformFlag::RotateY ? &transform->rotation.y : nullptr,
										transform->transformFlag & TransformFlag::RotateZ ? &transform->rotation.z : nullptr,
										nullptr, "Rotation", propGuiID);
						}

						if (transform->transformFlag & TransformFlag::ScaleXYZ)
						{
							ImGuiVector(transform->transformFlag & TransformFlag::ScaleX ? &transform->scale.x : nullptr,
										transform->transformFlag & TransformFlag::ScaleY ? &transform->scale.y : nullptr,
										transform->transformFlag & TransformFlag::ScaleZ ? &transform->scale.z : nullptr,
										nullptr, "Scale", propGuiID);
						}

						if (transform->transformFlag & TransformFlag::SizeXYZ)
						{
							ImGuiVector(transform->transformFlag & TransformFlag::SizeX ? &transform->size.x : nullptr,
										transform->transformFlag & TransformFlag::SizeY ? &transform->size.y : nullptr,
										transform->transformFlag & TransformFlag::SizeZ ? &transform->size.z : nullptr, nullptr, "Size", propGuiID);
						}

						if (transform->transformFlag & TransformFlag::RectSizeXY)
						{
							ImGuiVector(transform->transformFlag & TransformFlag::RectSizeX ? &transform->size.x : nullptr,
										transform->transformFlag & TransformFlag::RectSizeY ? &transform->size.y : nullptr,
										nullptr, nullptr, "Size", propGuiID);
						}

						if (transform->transformFlag & TransformFlag::RectAnchorn)
						{
							ImGuiVector(transform->transformFlag & TransformFlag::RectAnchorn ? &transform->offset.x : nullptr,
										transform->transformFlag & TransformFlag::RectAnchorn ? &transform->offset.y : nullptr,
										nullptr, nullptr, "Offset", propGuiID);
						}
					}
					else
					{
						ImGui::Text(prop.propName.c_str());
						ImGui::NextColumn();

						ImGui::SetNextItemWidth(-1);

						if (prop.type == Type::Boolean)
						{
							if (ImGui::Checkbox(propGuiID, (bool*)prop.value))
							{
								prop.changed = true;
							}
						}
						else
						if (prop.type == Type::Integer)
						{
							if (ImGui::InputInt(propGuiID, (int*)prop.value))
							{
								prop.changed = true;
							}
						}
						else
						if (prop.type == Type::Float)
						{
							if (ImGui::InputFloat(propGuiID, (float*)prop.value))
							{
								prop.changed = true;
							}
						}
						else
						if (prop.type == Type::String)
						{
							eastl::string* str = (eastl::string*)prop.value;

							struct Funcs
							{
								static int ResizeCallback(ImGuiInputTextCallbackData* data)
								{
									if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
									{
										eastl::string* str = (eastl::string*)data->UserData;
										str->resize(data->BufSize + 1);
										data->Buf = str->begin();
									}
									return 0;
								}
							};

							if (ImGui::InputText(propGuiID, str->begin(), (size_t)str->size() + 1, ImGuiInputTextFlags_CallbackResize, Funcs::ResizeCallback, (void*)str))
							{
								prop.changed = true;
							}
						}
						else
						if (prop.type == Type::FileName)
						{
							eastl::string* str = (eastl::string*)prop.value;

							StringUtils::Printf(propGuiID, 256, "%s###%s%s%i", str->c_str()[0] ? str->c_str() : "File not set", categoriesData[j].name.c_str(), guiID, i);

							if (ImGui::Button(propGuiID, ImVec2(ImGui::GetContentRegionAvail().x - 30.0f, 0.0f)))
							{
								const char* fileName = editor.OpenFileDialog("Any file", nullptr, true);

								if (fileName)
								{
									char relativeName[512];
									StringUtils::GetCropPath(Oak::root.GetRootPath(), fileName, relativeName, 512);

									str->assign(relativeName);
									prop.changed = true;
								}
							}

							if (str->c_str()[0] && (ImGui::IsItemActive() || ImGui::IsItemHovered()))
							{
								ImGui::SetTooltip(str->c_str());
							}

							ImGui::SameLine();

							StringUtils::Printf(propGuiID, 256, "Del###%s%s%iDel", categoriesData[j].name.c_str(), guiID, i);

							if (ImGui::Button(propGuiID, ImVec2(30.0f, 0.0f)))
							{
								str->assign("");
								prop.changed = true;
							}
						}
						else
						if (prop.type == Type::Color)
						{
							if (ImGui::ColorEdit4(propGuiID, (float*)prop.value))
							{
								prop.changed = true;
							}
						}
						else
						if (prop.type == Type::Enum)
						{
							int value = *((int*)prop.value);
							int index = 0;

							MetaDataEnum& enumData = enums[prop.defvalue.enumIndex];

							for (int i = 0; i < enumData.values.size(); i++)
							{
								if (enumData.values[i] == value)
								{
									index = i;
									break;
								}
							}

							if (enumData.enumList.empty())
							{
								int count = 0;

								for (int i = 0; i < enumData.names.size(); i++)
								{
									count += (int)enumData.names[i].size() + 1;
								}

								enumData.enumList.resize(count + 1);

								int index = 0;

								for (int i = 0; i < enumData.names.size(); i++)
								{
									int sz = (int)enumData.names[i].size() + 1;
									memcpy(&enumData.enumList[index], enumData.names[i].c_str(), sz);
									index += sz;
								}
							}

							if (ImGui::Combo(propGuiID, &index, enumData.enumList.c_str()))
							{
								*((int*)prop.value) = enumData.values[index];
								prop.changed = true;
							}
						}
						else
						if (prop.type == Type::EnumString)
						{
							//FIX ME!!!
						}
						else
						if (prop.type == Type::AssetTexture)
						{
							AssetTextureRef* ref = reinterpret_cast<AssetTextureRef*>(prop.value);

							if (ref->Get())
							{
								ImGui::BeginGroup();

								ref->ImGuiImage(120.0f);

								if (ref->Get() && (ImGui::IsItemActive() || ImGui::IsItemHovered()))
								{
									ImGui::SetTooltip(ref->Get()->GetPath().c_str());
								}

								StringUtils::Printf(propGuiID, 256, "Delete###%s%s%iDel", categoriesData[j].name.c_str(), guiID, i);

								if (ImGui::Button(propGuiID, ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
								{
									*ref = AssetTextureRef();
									prop.changed = true;
								}

								ImGui::EndGroup();
							}
							else
							{
								StringUtils::Printf(propGuiID, 256, "%s###%s%s%i", "None", categoriesData[j].name.c_str(), guiID, i);

								if (ImGui::Button(propGuiID, ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
								{
								}
							}

							if (ImGui::BeginDragDropTarget())
							{
								const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_ASSET_TEX", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

								if (payload)
								{
									AssetTextureRef* assetRef = reinterpret_cast<AssetTextureRef**>(payload->Data)[0];

									*ref = *assetRef;
									prop.changed = true;
								}
							}
						}
						else
						if (prop.type == Type::AssetAnimGraph2D)
						{
							AssetAnimGraph2DRef* ref = reinterpret_cast<AssetAnimGraph2DRef*>(prop.value);

							StringUtils::Printf(propGuiID, 256, "%s###%s%s%i", ref->Get() ? ref->Get()->GetName().c_str() : "None", categoriesData[j].name.c_str(), guiID, i);

							if (ImGui::Button(propGuiID, ImVec2(ImGui::GetContentRegionAvail().x - 30.0f, 0.0f)))
							{
							}

							if (ref->Get() && (ImGui::IsItemActive() || ImGui::IsItemHovered()))
							{
								ImGui::SetTooltip(ref->Get()->GetPath().c_str());
							}

							if (ImGui::BeginDragDropTarget())
							{
								const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_ASSET_ANIM_GRAPH_2D", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

								if (payload)
								{
									AssetAnimGraph2DRef* assetRef = reinterpret_cast<AssetAnimGraph2DRef**>(payload->Data)[0];

									*ref = *assetRef;
									prop.changed = true;
								}
							}

							ImGui::SameLine();

							StringUtils::Printf(propGuiID, 256, "Del###%s%s%iDel", categoriesData[j].name.c_str(), guiID, i);

							if (ImGui::Button(propGuiID, ImVec2(30.0f, 0.0f)))
							{
								*ref = AssetAnimGraph2DRef();
								prop.changed = true;
							}
						}
						else
						if (prop.type == Type::SceneEntity)
						{
							SceneEntityRef* ref = reinterpret_cast<SceneEntityRef*>(prop.value);
							StringUtils::Printf(propGuiID, 256, "%s###%s%s%i", ref->entity ? ref->entity->GetName() : "None", categoriesData[j].name.c_str(), guiID, i);

							if (ImGui::Button(propGuiID, ImVec2(ImGui::GetContentRegionAvail().x - 30.0f, 0.0f)))
							{
							}

							if (ImGui::BeginDragDropTarget())
							{
								const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TREENODE", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

								if (payload)
								{
									uint64_t temp = *((uint64_t*)payload->Data);
									SceneEntity* entity = (SceneEntity*)temp;

									ref->entity = entity;
									ref->uid = entity->GetUID();

									prop.changed = true;
								}
							}

							ImGui::SameLine();

							StringUtils::Printf(propGuiID, 256, "Del###%s%s%iDel", categoriesData[j].name.c_str(), guiID, i);

							if (ImGui::Button(propGuiID, ImVec2(30.0f, 0.0f)))
							{
								ref->entity = nullptr;
								ref->uid = 0;

								prop.changed = true;
							}
						}

						ImGui::NextColumn();
					}
				}
			}
		}
	}

	bool MetaData::IsValueWasChanged()
	{
		bool res = false;

		for (auto& prop : properties)
		{
			if (prop.type == Type::Array)
			{
				res |= prop.adapter->GetMetaData()->IsValueWasChanged();
			}
			else
			{
				res |= prop.changed;
				prop.changed = false;
			}
		}

		return res;
	}
	#endif
}