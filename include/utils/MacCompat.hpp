#pragma once

// Contournements pour macOS (Geode 6.0.0-beta.3 / GD 2.2091).
//
// Les bindings macOS de GD 2.2091 n'ont PAS d'adresse pour certaines fonctions
// ni pour le destructeur de certaines classes. Résultat à l'édition de liens :
//   - "Undefined symbols" pour ces fonctions,
//   - "typeinfo for X" introuvable pour les classes sans destructeur macOS,
//     typeinfo requis par m_fields (typeid) et typeinfo_cast.
// Ce fichier fournit des remplacements qui n'ont besoin d'aucun de ces symboles.

#include <Geode/Geode.hpp>
#include <typeinfo>
#include <string>

namespace tinker::utils::compat {

    // Équivalent de m_fields.self(), mais sans typeid(Base).
    // Les champs sont stockés dans un CCObject attaché au noeud (setUserObject),
    // donc libérés automatiquement avec le noeud (pas de pointeur périmé).
    template <class F>
    struct FieldsHolder : cocos2d::CCObject {
        F m_data;
    };

    template <class F>
    F* nodeFields(cocos2d::CCNode* node) {
        // typeid(F) concerne notre propre struct : son typeinfo est émis ici.
        std::string key = std::string("tinker-fields/") + typeid(F).name();
        auto holder = static_cast<FieldsHolder<F>*>(node->getUserObject(key));
        if (!holder) {
            auto created = new FieldsHolder<F>();
            created->autorelease();
            node->setUserObject(key, created);
            holder = created;
        }
        return &holder->m_data;
    }

    // EditorUI::deleteObject n'a pas d'adresse macOS : copie du code inline.
    inline void deleteObject(EditorUI* ui, GameObject* object, bool noUndo) {
        if (!object) return;
        object->m_wasSelected = object->m_isSelected;
        ui->m_editorLayer->removeObject(object, noUndo);
        ui->deactivateRotationControl();
        ui->deactivateScaleControl();
        ui->deactivateTransformControl();
    }

    // EditorUI::playCircleAnim n'a pas d'adresse macOS : copie du code inline.
    inline void playCircleAnim(EditorUI* ui, cocos2d::CCPoint position, float radius, float duration) {
        auto circleWave = CCCircleWave::create(1.f, radius, duration, false, false);
        circleWave->m_color.r = 255;
        circleWave->m_color.g = 255;
        circleWave->m_color.b = 255;
        circleWave->setPosition(position);
        ui->addChild(circleWave, 0);
        circleWave->m_circleMode = CircleMode::Outline;
    }

    // LevelSettingsObject::getIDName n'a pas d'adresse macOS.
    // Sur macOS on renvoie une chaîne vide (= pas de nom affiché).
    inline std::string getIDName(LevelSettingsObject* settings, int id, int type) {
#ifdef GEODE_IS_MACOS
        (void)settings; (void)id; (void)type;
        return std::string();
#else
        return settings->getIDName(id, type);
#endif
    }
}
